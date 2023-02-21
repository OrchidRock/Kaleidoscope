#ifndef __JIT_H__
#define __JIT_H__

#include "KaleidoscopeJIT.h"
#include "codegen.h"

using namespace llvm;
using namespace llvm::orc;

class JITVisitor : public CodeGenVisitor {
  std::unique_ptr<KaleidoscopeJIT> TheJIT;
public:
  JITVisitor(std::unique_ptr<LLVMContext> C, std::unique_ptr<Module> M,
                  int OptLevel) 
          :CodeGenVisitor(std::move(C), std::move(M), OptLevel){
    TheJIT = ExitOnErr(KaleidoscopeJIT::Create());
    TheModule->setDataLayout(TheJIT->getDataLayout());
  }
  Function* visit(FunctionAST&) override;

  void InitializeModuleAndPassManager();
}; 

#endif
