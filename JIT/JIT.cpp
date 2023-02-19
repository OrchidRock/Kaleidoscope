#include "../include/JIT.h"
#include "../include/codegen.h"

void JITVisitor::InitializeModuleAndPassManager() {
  // Open a new module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("my cool jit", *TheContext);
  
  Builder = std::make_unique<IRBuilder<>>(*TheContext);
  
  InitOptimPassManager();
  
  TheModule->setDataLayout(TheJIT->getDataLayout());
}

Function* JITVisitor::visit(FunctionAST& Node){
    
    auto &P =  *(Node.Proto);
    auto *FnIR = CodeGenVisitor::visit(Node);
    if(FnIR)
        FnIR->print(errs());
    
    if (P.getName() == "__anon_expr"){
      // Create a ResourceTracker to track JIT'd memory allocated to our
      // anonymous expression -- that way we can free it after executing.
      auto RT = TheJIT->getMainJITDylib().createResourceTracker();

      auto TSM = ThreadSafeModule(std::move(TheModule), std::move(TheContext));
      ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
      InitializeModuleAndPassManager();

      // Search the JIT for the __anon_expr symbol.
      auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anon_expr"));

      // Get the symbol's address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function.
      double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
      fprintf(stderr, "Evaluated to %f\n", FP());

      // Delete the anonymous expression module from the JIT.
      ExitOnErr(RT->remove());
      
      return FnIR;   
    }
    
    ExitOnErr(TheJIT->addModule(
          ThreadSafeModule(std::move(TheModule), std::move(TheContext))));
    InitializeModuleAndPassManager();

    return FnIR;
}
