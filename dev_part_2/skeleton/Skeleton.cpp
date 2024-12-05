#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <queue>

using namespace llvm;

namespace {
    struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    private:
        // Helper function to trace a value's definition
        void traceDefinition(Value* V, std::set<Value*>& visited) {
            if (!V || visited.count(V))
                return;
                
            visited.insert(V);
            
            if (Instruction* I = dyn_cast<Instruction>(V)) {
                errs() << "Found definition at: " << *I << "\n";
                
                // Recursively trace operands
                for (Use& U : I->operands()) {
                    Value* operand = U.get();
                    traceDefinition(operand, visited);
                }
            }
            else if (Argument* arg = dyn_cast<Argument>(V)) {
                errs() << "Variable comes from function argument: " << *arg << " in function " 
                       << arg->getParent()->getName() << "\n";
            }
            else if (GlobalVariable* GV = dyn_cast<GlobalVariable>(V)) {
                errs() << "Variable comes from global: " << *GV << "\n";
            }
            else if (Constant* C = dyn_cast<Constant>(V)) {
                errs() << "Variable comes from constant: " << *C << "\n";
            }
        }

        // Helper function to handle PHI nodes
        void handlePHINode(PHINode* phi, std::set<Value*>& visited) {
            errs() << "Found PHI node: " << *phi << "\n";
            for (unsigned i = 0; i < phi->getNumIncomingValues(); ++i) {
                Value* incomingValue = phi->getIncomingValue(i);
                BasicBlock* incomingBlock = phi->getIncomingBlock(i);
                errs() << "  Incoming value from block " << incomingBlock->getName() << ": "
                       << *incomingValue << "\n";
                traceDefinition(incomingValue, visited);
            }
        }

        // Helper function to handle function calls
        void handleCall(CallInst* call, std::set<Value*>& visited) {
            Function* calledFunc = call->getCalledFunction();
            if (!calledFunc) {
                errs() << "Called function is indirect or inline assembly\n";
                return;
            }

            errs() << "Function call to: " << calledFunc->getName() << "\n";
            
            // If the function is available, analyze its return value
            if (!calledFunc->isDeclaration()) {
                for (BasicBlock& BB : *calledFunc) {
                    for (Instruction& I : BB) {
                        if (ReturnInst* ret = dyn_cast<ReturnInst>(&I)) {
                            if (Value* retVal = ret->getReturnValue()) {
                                errs() << "Return value defined at: " << *retVal << "\n";
                                traceDefinition(retVal, visited);
                            }
                        }
                    }
                }
            }
        }

    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
            errs() << "Running Variable Trace Pass\n";
            
            for (Function& F : M) {
                for (BasicBlock& BB : F) {
                    for (Instruction& I : BB) {
                        std::set<Value*> visited;
                        
                        // Handle different types of instructions
                        if (PHINode* phi = dyn_cast<PHINode>(&I)) {
                            handlePHINode(phi, visited);
                        }
                        else if (CallInst* call = dyn_cast<CallInst>(&I)) {
                            handleCall(call, visited);
                        }
                        else {
                            traceDefinition(&I, visited);
                        }
                    }
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
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}