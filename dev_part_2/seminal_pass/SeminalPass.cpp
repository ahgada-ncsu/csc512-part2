#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include <map>
#include <string>
#include <set>
#include <fstream>
#include <vector>
#include <sstream>

using namespace llvm;

namespace {
    struct SeminalPass : public PassInfoMixin<SeminalPass> {
    private:
        std::map<Value*, std::string> varNames;
        std::map<Value*, DILocalVariable*> debugVars;

        void printFunctionHeader(Function& F) {
            DISubprogram* SP = F.getSubprogram();
            unsigned line = SP ? SP->getLine() : 0;
            
            errs() << "Analyzing function " << F.getName() 
                << " at line " << line << " (";
            
            // Print arguments
            bool first = true;
            for (auto& Arg : F.args()) {
                if (!first) errs() << ", ";
                if (DILocalVariable* DV = findArgDebugInfo(&Arg)) {
                    errs() << DV->getName();
                    first = false;
                }
            }
            
            errs() << ")\n";
        }

        DILocalVariable* findArgDebugInfo(Argument* Arg) {
            Function* F = Arg->getParent();
            if (!F->getSubprogram()) return nullptr;
            
            unsigned ArgNo = Arg->getArgNo();
            
            for (BasicBlock& BB : *F) {
                for (Instruction& I : BB) {
                    if (DbgDeclareInst* DDI = dyn_cast<DbgDeclareInst>(&I)) {
                        DILocalVariable* DV = DDI->getVariable();
                        if (DV && DV->getArg() == ArgNo + 1) {
                            return DV;
                        }
                    }
                }
            }
            return nullptr;
        }

        void printDbgValueInfo(const DbgDeclareInst* DDI) {
            if (!DDI) return;
            
            DILocalVariable* Var = DDI->getVariable();
            DILocation* Loc = DDI->getDebugLoc().get();
            if (Var && Loc) {
                varNames[DDI->getAddress()] = Var->getName().str();
                debugVars[DDI->getAddress()] = Var;
                errs() << Var->getName().str() << " defined at line " << Loc->getLine() << "\n";
            }
        }

        std::string getVariableName(Value* V) {
            if (varNames.count(V)) {
                return varNames[V];
            }
            
            // Try to get name from debug info for arrays
            if (GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(V)) {
                Value* PtrOp = GEP->getPointerOperand();
                if (debugVars.count(PtrOp)) {
                    return debugVars[PtrOp]->getName().str();
                }
            }
            
            return "";
        }

        // void traceStoreValue(StoreInst* SI) {
        //     if (!SI->getDebugLoc()) return;
            
        //     Value* PtrOp = SI->getPointerOperand();
        //     Value* ValOp = SI->getValueOperand();
            
        //     std::string varName = getVariableName(PtrOp);
        //     if (!varName.empty()) {
        //         errs() << varName << " gets value at line " << SI->getDebugLoc().getLine() << "\n";
        //     }
        // }

        void traceStoreValue(StoreInst* SI) {
            if (!SI->getDebugLoc()) return;
            
            Value* PtrOp = SI->getPointerOperand();
            Value* ValOp = SI->getValueOperand();
            
            std::string varName = getVariableName(PtrOp);
            if (!varName.empty()) {
                errs() << varName << " gets value at line " << SI->getDebugLoc().getLine() << " || ";
                
                const DebugLoc &DL = SI->getDebugLoc();
                DILocation* Loc = DL.get();
                if (Loc) {
                    StringRef File = Loc->getFilename();
                    unsigned Line = DL.getLine();
                    
                    // Read the source file
                    std::ifstream sourceFile(File.str());
                    if (sourceFile.is_open()) {
                        std::string sourceLine;
                        unsigned currentLine = 1;
                        
                        // Read until we find the line we want
                        while (std::getline(sourceFile, sourceLine) && currentLine < Line) {
                            currentLine++;
                        }
                        
                        if (currentLine == Line) {
                            errs() << "Code: " << sourceLine << "\n";
                        }
                        
                        sourceFile.close();
                    }
                }
            }
        }

        std::string getArgValue(Value* Arg, Function* CalledF = nullptr) {
            // Handle string literals
            if (GEPOperator* GEP = dyn_cast<GEPOperator>(Arg)) {
                if (GlobalVariable* GV = dyn_cast<GlobalVariable>(GEP->getPointerOperand())) {
                    if (GV->hasInitializer()) {
                        if (ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer())) {
                            if (CDA->isCString()) {
                                return "\"" + CDA->getAsCString().str() + "\"";
                            }
                        }
                    }
                }
            }
            
            // Handle array variables
            if (GetElementPtrInst* GEP = dyn_cast<GetElementPtrInst>(Arg)) {
                Value* PtrOp = GEP->getPointerOperand();
                if (AllocaInst* AI = dyn_cast<AllocaInst>(PtrOp)) {
                    if (debugVars.count(AI)) {
                        return debugVars[AI]->getName().str();
                    }
                }
            }
            
            // Handle regular variables
            if (LoadInst* LI = dyn_cast<LoadInst>(Arg)) {
                std::string varName = getVariableName(LI->getPointerOperand());
                if (!varName.empty()) {
                    return varName;
                }
            }
            
            std::string varName = getVariableName(Arg);
            if (!varName.empty()) {
                return varName;
            }
            
            return "";
        }

        void handleFunctionCall(CallInst* CI) {
            Function* F = CI->getCalledFunction();
            if (!F || F->getName().startswith("llvm.dbg")) return;
            
            if (CI->getDebugLoc()) {
                std::string fnName = F->getName().str();
                errs() << "Function call to " << fnName
                       << " at line " << CI->getDebugLoc().getLine()
                       << " with arguments: (";
                
                bool first = true;
                for (Use &U : CI->args()) {
                    if (!first) errs() << ", ";
                    first = false;
                    
                    std::string argValue = getArgValue(U.get(), F);
                    if (argValue.empty()) {
                        // Special handling for printf format strings
                        if (fnName == "printf" && first) {
                            if (GEPOperator* GEP = dyn_cast<GEPOperator>(U.get())) {
                                if (GlobalVariable* GV = dyn_cast<GlobalVariable>(GEP->getPointerOperand())) {
                                    if (GV->hasInitializer()) {
                                        if (ConstantDataArray* CDA = dyn_cast<ConstantDataArray>(GV->getInitializer())) {
                                            if (CDA->isCString()) {
                                                errs() << "\"%s\\n\"";
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        errs() << "unknown";
                    } else {
                        errs() << argValue;
                    }
                }
                errs() << ")\n";
            }
        }

        void processInstruction(Instruction* I) {
            if (DbgDeclareInst* DDI = dyn_cast<DbgDeclareInst>(I)) {
                printDbgValueInfo(DDI);
            }
            else if (StoreInst* SI = dyn_cast<StoreInst>(I)) {
                traceStoreValue(SI);
            }
            else if (CallInst* CI = dyn_cast<CallInst>(I)) {
                handleFunctionCall(CI);
            }
        }

        std::vector<unsigned int> targetLines = {11, 12, 13}; // Example line numbers
        std::map<unsigned int, std::set<std::string>> lineToVars;

        // Helper function to find debug declare instruction for a value
        const DbgDeclareInst* findDbgDeclare(const Value *V) {
            const Function *F = nullptr;
            if (const Instruction *I = dyn_cast<Instruction>(V))
                F = I->getFunction();
            else if (const Argument *Arg = dyn_cast<Argument>(V))
                F = Arg->getParent();
            
            if (!F) return nullptr;

            for (const BasicBlock &BB : *F) {
                for (const Instruction &I : BB) {
                    if (const DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(&I)) {
                        if (DDI->getAddress() == V)
                            return DDI;
                    }
                }
            }
            return nullptr;
        }

        void getVariableNamesAtLine(const Instruction &I) {
            const DebugLoc &DL = I.getDebugLoc();
            if (!DL) return;

            unsigned int currentLine = DL.getLine();
            
            // Check if this line is one we're interested in
            if (std::find(targetLines.begin(), targetLines.end(), currentLine) == targetLines.end())
                return;

            // Create set for this line if it doesn't exist
            auto &varNames = lineToVars[currentLine];

            // Check if this is a load instruction
            if (const LoadInst *LI = dyn_cast<LoadInst>(&I)) {
                if (const Value *V = LI->getPointerOperand()) {
                    if (const DbgDeclareInst *DDI = findDbgDeclare(V)) {
                        if (DILocalVariable *DIVar = DDI->getVariable()) {
                            varNames.insert(DIVar->getName().str());
                        }
                    }
                }
            }

            // Check if this is a store instruction
            if (const StoreInst *SI = dyn_cast<StoreInst>(&I)) {
                if (const Value *V = SI->getPointerOperand()) {
                    if (const DbgDeclareInst *DDI = findDbgDeclare(V)) {
                        if (DILocalVariable *DIVar = DDI->getVariable()) {
                            varNames.insert(DIVar->getName().str());
                        }
                    }
                }
            }

            // Check debug info for the instruction itself
            if (const DbgValueInst *DVI = dyn_cast<DbgValueInst>(&I)) {
                if (DILocalVariable *DIVar = DVI->getVariable()) {
                    varNames.insert(DIVar->getName().str());
                }
            }

            // Check all operands
            for (const Use &U : I.operands()) {
                if (const Value *V = U.get()) {
                    if (const AllocaInst *AI = dyn_cast<AllocaInst>(V)) {
                        if (const DbgDeclareInst *DDI = findDbgDeclare(AI)) {
                            if (DILocalVariable *DIVar = DDI->getVariable()) {
                                varNames.insert(DIVar->getName().str());
                            }
                        }
                    }
                }
            }
        }

        std::vector<unsigned int> readBranchInfo() {
            std::ifstream file("branch_info.txt");
            std::set<unsigned int> uniqueLines;  // Using set for unique numbers
            std::string line;

            if (!file.is_open()) {
                errs() << "Error: Could not open branch_info.txt\n";
                return std::vector<unsigned int>();
            }

            while (std::getline(file, line)) {
                // Skip empty lines
                if (line.empty()) continue;

                // Find position of first comma
                size_t pos = line.find(',');
                if (pos == std::string::npos) continue;

                // Extract the part after the comma and trim whitespace
                std::string numberPart = line.substr(pos + 1);
                pos = numberPart.find(',');
                if (pos == std::string::npos) continue;

                // Extract the number and trim whitespace
                std::string numberStr = numberPart.substr(0, pos);
                // Remove leading/trailing spaces
                numberStr.erase(0, numberStr.find_first_not_of(" "));
                numberStr.erase(numberStr.find_last_not_of(" ") + 1);

                // Convert to integer and add to set
                unsigned int lineNum = std::stoi(numberStr);
                uniqueLines.insert(lineNum);
            }

            file.close();

            // Convert set to vector for return
            return std::vector<unsigned int>(uniqueLines.begin(), uniqueLines.end());
        }

    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {

            // read text file branch_info.txt

            targetLines = readBranchInfo();

            for (Function &F : M) {
                if (F.isDeclaration())
                    continue;
                    
                for (BasicBlock &BB : F) {
                    for (Instruction &I : BB) {
                        getVariableNamesAtLine(I);
                    }
                }
            }
            
            // Print results for each line number
            for (unsigned int line : targetLines) {
                errs() << "Variables at line " << line << ": ";
                if (lineToVars.count(line) && !lineToVars[line].empty()) {
                    bool first = true;
                    for (const auto &varName : lineToVars[line]) {
                        if (!first) errs() << ",";
                        errs() << varName;
                        first = false;
                    }
                }
                errs() << "\n";
            }

            errs() << "\n\n\n";
            
            // Second pass: Function trace analysis
            for (Function& F : M) {
                if (!F.isDeclaration()) {
                    printFunctionHeader(F);
                    
                    for (BasicBlock& BB : F) {
                        for (Instruction& I : BB) {
                            processInstruction(&I);
                        }
                    }
                    errs() << "\n";
                }
            }
            
            return PreservedAnalyses::all();
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Variable Trace Pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SeminalPass());
                });
        }
    };
}