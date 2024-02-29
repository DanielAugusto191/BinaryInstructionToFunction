#ifndef MAIN_H
#define MAIN_H

#include "llvm/IR/PassManager.h"

namespace llvm {
	struct itf: public PassInfoMixin<itf>{
		public:
			PreservedAnalyses run(Module &, ModuleAnalysisManager &);
	};
}

#endif
