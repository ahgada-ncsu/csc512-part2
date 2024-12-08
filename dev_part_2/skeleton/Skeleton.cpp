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

using namespace llvm;

namespace {


struct BranchInfo {
    std::string filepath;
    int branch_id;
    unsigned int src_lno;
    unsigned int dest_lno;
};
std::vector<BranchInfo> branchInfos;

FunctionCallee CreateBranchFunction(Function &F) {
    LLVMContext &func_context = F.getContext();
    std::vector<Type*> parameters = {
        Type::getInt32Ty(func_context),
    };
    
    FunctionType *func_type = FunctionType::get(Type::getVoidTy(func_context), parameters, false);

    FunctionCallee func_callee = F.getParent()->getOrInsertFunction("LogBranch", func_type);

    return func_callee;
}

FunctionCallee CreatePointerFunction(Function &F) {
    LLVMContext &func_context = F.getContext();
    std::vector<Type*> ParamTypes = {
        Type::getInt8PtrTy(func_context),
    };

    // Define the function type
    FunctionType *func_type = FunctionType::get(Type::getVoidTy(func_context), ParamTypes, false);

    // Check if the function exists within the module that F belongs to, and if not, insert it
    FunctionCallee func_callee = F.getParent()->getOrInsertFunction("LogPointer", func_type);
    
    return func_callee;
}


struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {

        int branch_id_counter = 1;
        std::ofstream file("branch_info.txt", std::ios::out | std::ios::trunc);
        for (auto &F : M.functions()) {

            LLVMContext &func_context = F.getContext();
            FunctionCallee branch_func_callee = CreateBranchFunction(F);
            FunctionCallee pointer_func_callee = CreatePointerFunction(F);

            for(auto &B:F) {
                for(auto & I:B) {
                    
                    auto *branch_instruction = dyn_cast<BranchInst>(&I);

                    if(branch_instruction && branch_instruction->isConditional()) {

                        DILocation *source_location = branch_instruction->getDebugLoc(); //Maybe using I.getDebugLoc()

                        if(source_location) {

                            std::string source_file_name = source_location->getFilename().str();

                            unsigned int source_line_number = source_location->getLine();

                            for (unsigned int ii = 0; ii < branch_instruction->getNumSuccessors(); ++ii) {
                                
                                BasicBlock *successor = branch_instruction->getSuccessor(ii);

                                if(successor && !successor->empty()) {

                                    DILocation *successor_location = successor->front().getDebugLoc();

                                    unsigned int target_line_number = successor_location->getLine();

                                    branchInfos.push_back({source_file_name, branch_id_counter, source_line_number, target_line_number});

                                    IRBuilder<> Builder(func_context);

                                    Builder.SetInsertPoint(&(successor->front()));

                                    Builder.CreateCall(branch_func_callee, {ConstantInt::get(Type::getInt32Ty(func_context), branch_id_counter)});

                                    branch_id_counter++;

                                }
                            }

                        }

                    }

                    auto *pointer_instruction = dyn_cast<CallInst>(&I);
                    
                    if( pointer_instruction){
                        
                        if(!pointer_instruction->getCalledFunction()){
                            
                            IRBuilder<> Builder(pointer_instruction);

                            Value *called_value = pointer_instruction->getCalledOperand();
                            
                            // if (called_value) {

                            //     errs() << "*funcptr_" << called_value << "\n";
                            // }

                            Builder.CreateCall(pointer_func_callee, called_value);
                        }
                        
                    }
                }
            }
            
        }

        for (const auto &branch : branchInfos) {
            file << "br_" << branch.branch_id << ": " << branch.filepath << ", " 
                << branch.src_lno << ", " << branch.dest_lno << "\n";
        }
        file.close();
        return PreservedAnalyses::all();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}