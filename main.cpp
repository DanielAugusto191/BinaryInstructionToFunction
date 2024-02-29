#include <iostream>
#include "llvm/IR/Argument.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/User.h"
#include "main.h"
#include <iterator>
#include <map>
#include <utility>

using namespace llvm;
PreservedAnalyses itf::run(Module &M, ModuleAnalysisManager &MAM){
	int i =0;
	std::set<Function *> FtoMap;	
	for(Function &F: M.getFunctionList()) FtoMap.insert(&F);
	for(Function *F: FtoMap){
		for(Instruction &I: instructions(F)){
			if(isa<BinaryOperator>(I)){
				typedef std::vector<std::pair<int,int> > ItoJ;
				DenseMap<Instruction *, ItoJ> mps; // On instruction I, substitute operator i with function argument j
				Instruction *newI = I.clone();
				// Make Function
				std::vector<Value *> vFArgsTypes;
				std::set<Value *> lookvFArgsTypes;
				int posK = 0;
				for(int i=0;i < newI->getNumOperands(); ++i){
					auto X = newI->getOperand(i);
					if(!isa<Constant>(X)){
						if(!lookvFArgsTypes.count(X)){
							lookvFArgsTypes.insert(X);
							vFArgsTypes.push_back(X);
							int posNew = vFArgsTypes.size()-1;
							auto itM = mps.insert({newI, ItoJ()});
							itM.first->second.push_back({i, posNew});
						}else{
							auto K = mps.find(newI);
							int posK = 0;
							for(auto &p: vFArgsTypes){
								if(p == X) break;
								posK++;
							}
							K->second.push_back({i,posK});
						}
					}
				}
				SmallVector<Type *> v;
				for(auto &p: vFArgsTypes) v.push_back(p->getType());

				FunctionType *fType = FunctionType::get(I.getType(),v, false);
				Function *newF = Function::Create(fType, Function::ExternalLinkage, I.getName().str(), M);
				auto T = new Argument(newI->getType(), "", newF);

				for(auto &e: mps){
					for(auto &k: e.second){
						e.first->setOperand(k.first, newF->getArg(k.second));
					}
				}


				// Make BasicBlock and add Inst
				BasicBlock *BB = BasicBlock::Create(M.getContext(), "BB", newF);

				BB->getInstList().push_front(newI);

				// Return type to Function
				ReturnInst::Create(M.getContext(), newI, BB);

				dbgs() << *newF;
			}
		}
	}
	return PreservedAnalyses::all();
}

bool registerPipeline(StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
	if(Name == "itf"){
		MPM.addPass(itf());
		return true;
	}
	return false;
}

PassPluginLibraryInfo instToFuncPluginInfo() {
	return {
		LLVM_PLUGIN_API_VERSION,
		"instToFunc",
		LLVM_VERSION_STRING,
		[](PassBuilder &PB){
			PB.registerPipelineParsingCallback(registerPipeline);
		}
	};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return instToFuncPluginInfo();
}
