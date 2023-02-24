#ifndef __CODEGEN_H__
#define __CODEGEN_H__
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/SourceMgr.h"

#include <memory>
#include "../include/AST.h"

using namespace llvm;

class CodeGenVisitor : public ASTVisitor {
    llvm::SourceMgr *SrcMgr = nullptr;
    int OptLevel = 0;
public:
    std::unique_ptr<LLVMContext> TheContext;
    std::unique_ptr<Module> TheModule;
    std::unique_ptr<IRBuilder<>> Builder;

    std::map<std::string, AllocaInst *> NamedValues;
    std::unique_ptr<legacy::FunctionPassManager> TheFPM;
    std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
    ExitOnError ExitOnErr;

    Function* getFunction(std::string);
    AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, StringRef VarName);
    
    CodeGenVisitor(llvm::SourceMgr *SrcMgr, std::unique_ptr<LLVMContext> C, 
                    std::unique_ptr<Module> M, int OptLevel)
            : SrcMgr(SrcMgr), OptLevel(OptLevel),
            TheContext(std::move(C)), TheModule(std::move(M)){
        // Create a new builder for the module.
        Builder = std::make_unique<IRBuilder<>>(*TheContext);
        InitOptimPassManager();    
    }
    
    CodeGenVisitor(std::unique_ptr<LLVMContext> C, std::unique_ptr<Module> M,
                    int OptLevel)
            : OptLevel(OptLevel), TheContext(std::move(C)), TheModule(std::move(M)) {
        // Create a new builder for the module.
        Builder = std::make_unique<IRBuilder<>>(*TheContext);
        InitOptimPassManager();    
    }
    
    Value* visit(NumberExprAST&) override;
    Value* visit(VariableExprAST&) override;
    Value* visit(UnaryExprAST&) override;
    Value* visit(BinaryExprAST&) override;
    Value* visit(CallExprAST&) override;
    Value* visit(IfExprAST&) override;
    Value* visit(ForExprAST&) override;
    Value* visit(VarExprAST&) override;
    Function* visit(PrototypeAST&) override;
    virtual Function* visit(FunctionAST&) override; 
    
    void InitOptimPassManager();

    Value *LogErrorV(llvm::SMLoc, const char *Str);
};



#endif
