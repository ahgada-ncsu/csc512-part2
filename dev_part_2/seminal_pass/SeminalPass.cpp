#include "sp.hpp"

using namespace llvm;

namespace {
    struct SeminalPass : public PassInfoMixin<SeminalPass> {
    private:
        std::map<Value*, std::string> varNames;
        std::map<Value*, DILocalVariable*> debugVars;
        string current_scope = "global";

        void analyzeGlobalVariables(Module &M) {
            for (GlobalVariable &GV : M.globals()) {
                if (DIGlobalVariableExpression* DIGVE = dyn_cast_or_null<DIGlobalVariableExpression>(
                        GV.getMetadata(LLVMContext::MD_dbg))) {
                    DIGlobalVariable *DGV = DIGVE->getVariable();
                    
                    var_map vm;
                    vm.name = DGV->getName().str();
                    vm.scope = "global";
                    vm.defined_at_line = DGV->getLine();
                    vm.gets_value_infos = std::vector<get_list>();
                    variable_infos.push_back(vm);
                    
                    // Track the name for later use
                    varNames[&GV] = DGV->getName().str();

                    // Check initializer
                    if (GV.hasInitializer()) {
                        // 
                    }
                }
            }
        }

        void printFunctionHeader(Function& F) {
            DISubprogram* SP = F.getSubprogram();
            unsigned line = SP ? SP->getLine() : 0;
        
            func_map fm;
            fm.line_num = line;
            fm.name = F.getName().str();
            fm.args = std::vector<param>();

            current_scope = F.getName().str();

            for (auto& Arg : F.args()) {
                if (DILocalVariable* DV = findArgDebugInfo(&Arg)) {
                    fm.args.push_back({Arg.getArgNo(), DV->getName().str()});
                } else {
                    // Handle case where debug info isn't available
                    // Use a default name based on argument position
                    std::string defaultName = "arg" + std::to_string(Arg.getArgNo());
                    fm.args.push_back({Arg.getArgNo(), defaultName});
                }
            }

            functions.push_back(fm);
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
            var_map vm;
            if (Var && Loc) {
                varNames[DDI->getAddress()] = Var->getName().str();
                debugVars[DDI->getAddress()] = Var;

                vm.name = Var->getName().str();
                vm.scope = current_scope;
                vm.defined_at_line = Loc->getLine();
                vm.gets_value_infos = std::vector<get_list>();
                variable_infos.push_back(vm);
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

        void traceStoreValue(StoreInst* SI) {
            if (!SI->getDebugLoc()) return;
            
            Value* PtrOp = SI->getPointerOperand();
            Value* ValOp = SI->getValueOperand();
            
            std::string varName = getVariableName(PtrOp);
            if (!varName.empty()) {
                // errs() << varName << " gets value at line " << SI->getDebugLoc().getLine() << " || ";
                
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
                            // errs() << "Code: " << sourceLine << "\n";

                            int v = find_variable_index_in_variable_infos(varName, current_scope);
                            if (v != -1) {
                                var_map vm = variable_infos[v];
                                get_list gl;
                                gl.gets_at_line = Line;
                                line_map lm = variables_per_line[find_line_index_in_variables_per_line(Line)];
                                gl.vars = lm;
                                gl.vars.scope = current_scope;
                                if(lm.vars.size() > 1) {
                                    // type can be func or var
                                    if(find_function_index_in_function_calls_line(Line) != -1) {
                                        gl.type = "func"; // func, var, val, param
                                    } else {
                                        gl.type = "var"; // func, var, val, param
                                    }

                                }else{
                                    // type can be val or param
                                    if(find_function_index_in_functions_line(Line) != -1) {
                                        gl.type = "param"; // func, var, val, param
                                    } else {
                                        gl.type = "var"; // func, var, val, param
                                    }
                                }
                                vector<string> temp = split(sourceLine, '=');
                                gl.code = temp[1];
                                vm.gets_value_infos.push_back(gl);
                                variable_infos[v] = vm;
                            }

                        }
                        
                        sourceFile.close();
                    }
                }
            }
        }

        vector<string> split(string str, char delimiter) {
            vector<string> internal;
            stringstream ss(str); // Turn the string into a stream.
            string tok;
            
            while(getline(ss, tok, delimiter)) {
                internal.push_back(tok);
            }
            
            return internal;
        }

        std::string getArgValue(Value* Arg, Function* CalledF = nullptr) {
            // Handle string literals

            if (ConstantInt* CI = dyn_cast<ConstantInt>(Arg)) {
                return std::to_string(CI->getSExtValue());
            }

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
            func_call_map fcm;
            
            if (CI->getDebugLoc()) {
                fcm.name = F->getName().str();
                fcm.args = std::vector<param>();
                
                int ii = 0;
                for (Use &U : CI->args()) {
                    std::string argValue = getArgValue(U.get(), F);
                    if (argValue.empty()) {
                        fcm.args.push_back({-1, "unknown"});
                    } else {
                        fcm.args.push_back({ii, argValue});\
                        ii+=1;
                    }
                }
            }

            fcm.scope = current_scope;
            fcm.line = CI->getDebugLoc().getLine();
            function_calls.push_back(fcm);
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

        void trackGlobalVariables(Module &M) {
            for (GlobalVariable &GV : M.globals()) {
                if (DIGlobalVariableExpression* DIGVE = dyn_cast_or_null<DIGlobalVariableExpression>(
                        GV.getMetadata(LLVMContext::MD_dbg))) {
                    DIGlobalVariable *DGV = DIGVE->getVariable();
                    unsigned line = DGV->getLine();
                    lineToVars[line].insert(DGV->getName().str());
                }
            }
        }

        void getVariableNamesAtLine(const Instruction &I) {
            const DebugLoc &DL = I.getDebugLoc();
            if (!DL) return;

            unsigned int currentLine = DL.getLine();
            auto &varNames = lineToVars[currentLine];

            // Check for DbgDeclareInst directly
            if (const DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(&I)) {
                if (DILocalVariable *DIVar = DDI->getVariable()) {
                    varNames.insert(DIVar->getName().str());
                }
            }

            // Check if this is a load instruction
            if (const LoadInst *LI = dyn_cast<LoadInst>(&I)) {
                // Check for global variables
                if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(LI->getPointerOperand())) {
                    if (DIGlobalVariableExpression* DIGVE = dyn_cast_or_null<DIGlobalVariableExpression>(
                            GV->getMetadata(LLVMContext::MD_dbg))) {
                        DIGlobalVariable *DGV = DIGVE->getVariable();
                        varNames.insert(DGV->getName().str());
                    }
                }
                // Check for local variables
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
                // Check for global variables
                if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(SI->getPointerOperand())) {
                    if (DIGlobalVariableExpression* DIGVE = dyn_cast_or_null<DIGlobalVariableExpression>(
                            GV->getMetadata(LLVMContext::MD_dbg))) {
                        DIGlobalVariable *DGV = DIGVE->getVariable();
                        varNames.insert(DGV->getName().str());
                    }
                }
                // Check for local variables
                if (const Value *V = SI->getPointerOperand()) {
                    if (const DbgDeclareInst *DDI = findDbgDeclare(V)) {
                        if (DILocalVariable *DIVar = DDI->getVariable()) {
                            varNames.insert(DIVar->getName().str());
                        }
                    }
                }
            }

            // Add variables at their declaration points
            for (const Use &U : I.operands()) {
                if (const Value *V = U.get()) {
                    if (const AllocaInst *AI = dyn_cast<AllocaInst>(V)) {
                        if (const DbgDeclareInst *DDI = findDbgDeclare(AI)) {
                            if (DILocalVariable *DIVar = DDI->getVariable()) {
                                // Get the line number from the debug location of the alloca instruction
                                if (const DebugLoc &AllocaLoc = AI->getDebugLoc()) {
                                    lineToVars[AllocaLoc.getLine()].insert(DIVar->getName().str());
                                }
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

        // function that finds the index of variable in variable_infos with name=n and scope=s
        int find_variable_index_in_variable_infos(string n, string s) {
            for (int i = 0; i < variable_infos.size(); i++) {
                if (variable_infos[i].name == n && variable_infos[i].scope == s) {
                    return i;
                }
            }
            return -1;
        }

        // function that finds the index of line in variables_per_line with line_num=l
        int find_line_index_in_variables_per_line(int l) {
            for (int i = 0; i < variables_per_line.size(); i++) {
                if (variables_per_line[i].line_num == l) {
                    return i;
                }
            }
            return -1;
        }

        // function to find the index of function in functions with name=n
        int find_function_index_in_functions(string n) {
            for (int i = 0; i < functions.size(); i++) {
                if (functions[i].name == n) {
                    return i;
                }
            }
            return -1;
        }

        // function to find the index of function in function_calls with line=l
        int find_function_index_in_functions_line(int l) {
            for (int i = 0; i < functions.size(); i++) {
                if (functions[i].line_num == l) {
                    return i;
                }
            }
            return -1;
        }

        int find_function_index_in_function_calls(string n) {
            for (int i = 0; i < function_calls.size(); i++) {
                if (function_calls[i].name == n) {
                    return i;
                }
            }
            return -1;
        }

        // function to find the index of function in function_calls with line=l
        int find_function_index_in_function_calls_line(int l) {
            for (int i = 0; i < function_calls.size(); i++) {
                if (function_calls[i].line == l) {
                    return i;
                }
            }
            return -1;
        }

        void do_analysis(string var_name, string scope, vector<string> s, bool found=false) {
            // find the variable in variable_infos
            int v = find_variable_index_in_variable_infos(var_name, scope);
            var_map vm = variable_infos[v];
            bool done = false;
            errs() << "Analyzing variable: " << var_name << " at line " << vm.defined_at_line << " with scope: "<<vm.scope << "\n";

            // if line number of definition is in functions, then this is a function call
                // note down function parameter in question
                // look for where that function is called
                // look for the name of that parameter in the function call
                // find where that parameter gets its value from in the new scope
            
            for(auto &f: functions) {
                if(f.line_num == vm.defined_at_line) {
                    int fci = find_function_index_in_functions(f.name);
                    func_map fm = functions[fci];
                    int arg_index = 0;
                    for(auto &pa: fm.args) 
                        if(pa.name == var_name) {arg_index = pa.id;}
                    for (int i = 0; i < function_calls.size(); i++) {
                        if (function_calls[i].name == f.name) {
                            func_call_map fcm = function_calls[i];
                            fcm.args[arg_index].name;
                            // prevent infinte recursion
                            if(fcm.args[arg_index].name != var_name && fcm.scope != scope) {
                                do_analysis(fcm.args[arg_index].name, fcm.scope, s);
                                done = true;
                            }
                        }
                    }
                }
            }
            
            // check if it gets value from some other variable
                // loop through gets_value_infos
                // if it gets value from "val" end search and deem that branch to be not seminal
                // if it gets value from "func"
                    // note down func value in question
                    // if more variables are involved, then find their source
                    // if more variables are not involved, then check if function call is an input call
                        // if it is an input call, then find where that input gets its value from
                        // if it is not an input call, then find where that function is defined and repeat the process
                            // if it is, stop search and deem that branch to be seminal
                            // if it is not, then stop search and deem the branch to have no seminal value                
                // if it gets value from only some other "var", then recursively search where that "var" gets its value from
            
            if(done) return;

            bool found_val = false;

            // find where it gets value from
            for (auto &gl : vm.gets_value_infos) {
                errs()<<"analyzing line: "<<gl.code<<"\n";
                
                // check if there is a function call on the same line, and analyze each function.
                for(int i = 0; i < function_calls.size(); i++) {
                    if(function_calls[i].line == gl.gets_at_line) {
                        // check if the name is part of the input functions
                        string fname = function_calls[i].name;
                        if(fname == "getc"){
                            found_val=true;
                        }else if(fname == "fopen"){
                            found_val = true;
                        } else if(fname == "fread"){
                            found_val = true;
                        } else if(fname == "scanf"){
                            found_val = true;
                        }
                    }
                }

                if(gl.vars.vars.size() == 0) return;
                for (auto &va : gl.vars.vars) {
                    if(va.name != var_name) {
                        do_analysis(va.name, gl.vars.scope, s, found_val);
                    }
                }

            }

            if(found_val || found) {
                errs() << "Branch is seminal\n";
            }
        }


    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
             // Track global variables first
            trackGlobalVariables(M);

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

            for (const auto& lineEntry : lineToVars) {
                line_map lm;
                lm.line_num = lineEntry.first;
                lm.vars = std::vector<variable>();
                if (!lineEntry.second.empty()) {
                    for (const auto &varName : lineEntry.second) 
                        lm.vars.push_back({varName});
                }
                variables_per_line.push_back(lm);
            }
    
            // First analyze global variables
            analyzeGlobalVariables(M);
            
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

            vector<pair<int, string>> scope_map;
            for (auto &f: functions) {
                if (f.name.empty()) continue;  // Skip if name is empty
                scope_map.push_back({f.line_num, f.name});
            }

            // Sort scope_map by line numbers to ensure proper ordering
            std::sort(scope_map.begin(), scope_map.end());

            if (!scope_map.empty()) {  // Only proceed if we have valid scopes
                int minn = scope_map[0].first;
                int maxx = scope_map[scope_map.size()-1].first;
                
                for (int v = 0; v < variables_per_line.size(); v++) {
                    line_map &current_line = variables_per_line[v];
                    int ln = current_line.line_num;
                    
                    // Default to global scope
                    current_line.scope = "global";
                    
                    // Find appropriate scope
                    for (size_t i = 0; i < scope_map.size(); i++) {
                        if (ln >= scope_map[i].first && 
                            (i == scope_map.size()-1 || ln < scope_map[i+1].first)) {
                            current_line.scope = scope_map[i].second;
                            break;
                        }
                    }
                }
            }

            errs() << "Variable Trace Analysis\n";
            errs() << "------------------------\n\n";
            errs() << "Variables defined at each line\n";

            // print variables per line
            for (auto &vp : variables_per_line) {
                errs() << "Line: " << vp.line_num << "\n";
                errs() << "  Scope: " << vp.scope << "\n";
                for (auto &va : vp.vars) {
                    errs() << "  Variable: " << va.name << "\n";
                }
            }

            errs() << "\nFUNCTIONS\n";
            errs() << "---------\n\n";

            // print function info
            for (auto &fi : functions) {
                errs() << "Function: " << fi.name << " defined at line " << fi.line_num << "\n";
                for (auto &pa : fi.args) {
                    errs() << "  Argument: " << pa.name << " at position " << pa.id << "\n";
                }
            }

            errs() << "\nVARIABLES\n";
            errs() << "---------\n\n";

            // print variable info
            for (auto &vi : variable_infos) {
                errs() << "Variable: " << vi.name << " defined at line " << vi.defined_at_line << " with scope: "<<vi.scope << "\n";
                for (auto &gl : vi.gets_value_infos) {
                    errs() << "  Gets value at line " << gl.gets_at_line << " with type " << gl.type << " and code " << gl.code << "\n";
                    errs() << "    Variables on this line: \n";
                    for (auto &va : gl.vars.vars) {
                        errs() << "      " << va.name << " scope: "<<gl.vars.scope << "\n";
                    }
                }
            }

            errs() << "\nFUNCTION CALLS\n";
            errs() << "--------------\n\n";

            // print function call info
            for (auto &fci : function_calls) {
                errs() << "Function call: " << fci.name << " at line " << fci.line << " with scope: "<<fci.scope << "\n";
                for (auto &pa : fci.args) {
                    errs() << "  Argument: " << pa.name << " at position " << pa.id << "\n";
                }
            }

            vector<string> s;

            errs() << "\n\n\n";

            do_analysis("c", "func", s);


            
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