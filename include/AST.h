#ifndef __AST_H__
#define __AST_H__

#include <string>
#include <memory>
#include <vector>
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/SMLoc.h"

using namespace llvm;

class ExprAST;
class NumberExprAST;
class VariableExprAST;
class UnaryExprAST;
class BinaryExprAST;
class CallExprAST;
class IfExprAST;
class ForExprAST;
class VarExprAST;
class PrototypeAST;
class FunctionAST;

class ASTVisitor {
public:
    virtual Value* visit(ExprAST&) { return nullptr; }
    virtual Value* visit(NumberExprAST&) = 0;
    virtual Value* visit(VariableExprAST&) = 0;
    virtual Value* visit(UnaryExprAST&) = 0;
    virtual Value* visit(BinaryExprAST&) = 0;
    virtual Value* visit(CallExprAST&) = 0;
    virtual Value* visit(IfExprAST&) = 0;
    virtual Value* visit(ForExprAST&) = 0;
    virtual Value* visit(VarExprAST&) = 0;
    virtual Function* visit(PrototypeAST&) = 0;
    virtual Function* visit(FunctionAST&) = 0; 
    virtual ~ASTVisitor() {}
};

class ExprAST {
  llvm::SMLoc Loc;
public:
  ExprAST(llvm::SMLoc Loc) : Loc(Loc) {}

  virtual ~ExprAST() = default;
  virtual Value* accept(ASTVisitor &V) = 0;

  llvm::SMLoc getLocation() { return Loc; }
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
public:
  double Val;

  NumberExprAST(double Val, llvm::SMLoc Loc) : ExprAST(Loc),Val(Val) {}
  
  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
public:
  std::string Name;
  VariableExprAST(const std::string &Name, llvm::SMLoc Loc) 
          : ExprAST(Loc), Name(Name) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
  const std::string &getName() const { return Name; }
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
public:
  char Opcode;
  std::unique_ptr<ExprAST> Operand;

  UnaryExprAST(char Opcode, std::unique_ptr<ExprAST> Operand, llvm::SMLoc Loc)
      : ExprAST(Loc), Opcode(Opcode), Operand(std::move(Operand)) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
public:
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS, llvm::SMLoc Loc)
      : ExprAST(Loc), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
public:
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args,
              llvm::SMLoc Loc)
      : ExprAST(Loc), Callee(Callee), Args(std::move(Args)) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
public:
  std::unique_ptr<ExprAST> Cond, Then, Else;

  IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
            std::unique_ptr<ExprAST> Else, llvm::SMLoc Loc)
      : ExprAST(Loc), Cond(std::move(Cond)), Then(std::move(Then)), 
        Else(std::move(Else)) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
public:
  std::string VarName;
  std::unique_ptr<ExprAST> Start, End, Step, Body;

  ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
             std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
             std::unique_ptr<ExprAST> Body,
             llvm::SMLoc Loc)
      : ExprAST(Loc), VarName(VarName), Start(std::move(Start)), 
        End(std::move(End)), Step(std::move(Step)), Body(std::move(Body)) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
public:
  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
  std::unique_ptr<ExprAST> Body;

  VarExprAST(
      std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames,
      std::unique_ptr<ExprAST> Body, llvm::SMLoc Loc)
      : ExprAST(Loc), VarNames(std::move(VarNames)), 
        Body(std::move(Body)) {}

  Value* accept(ASTVisitor &V) override { return V.visit(*this); }
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes), as well as if it is an operator.
class PrototypeAST {
public:
  std::string Name;
  std::vector<std::string> Args;
  bool IsOperator;
  unsigned Precedence; // Precedence if a binary op.

  PrototypeAST(const std::string &Name, std::vector<std::string> Args,
               bool IsOperator = false, unsigned Prec = 0)
      : Name(Name), Args(std::move(Args)), IsOperator(IsOperator),
        Precedence(Prec) {}

  Function* accept(ASTVisitor &V) { return V.visit(*this); }
  
  std::string getName(){ return Name; }

  bool isUnaryOp() const { return IsOperator && Args.size() == 1; }
  bool isBinaryOp() const { return IsOperator && Args.size() == 2; }

  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size() - 1];
  }

  unsigned getBinaryPrecedence() const { return Precedence; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
public:
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}
  std::string getName() { return "__anon_expr"; }
  Function* accept(ASTVisitor &V) { return V.visit(*this); }
};

#endif
