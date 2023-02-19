#ifndef __PARSER_H__
#define __PARSER_H__

#include <memory>
#include <map>
#include "AST.h"
#include "lexer.h"
#include "codegen.h"

class Parser {
    Lexer* lexer = nullptr;
    ASTVisitor* Visitor = nullptr;
    bool IsJit = false;
public:
    
    Parser(Lexer* lexer, ASTVisitor* visitor, bool isJit = false)
            :lexer(lexer), Visitor(visitor), IsJit(isJit) {}

    int CurTok;
    int getNextToken();

    /// BinopPrecedence - This holds the precedence for each binary operator that is
    /// defined.
    static std::map<char, int> BinopPrecedence;
    static std::map<char, int> InitBinopPrecedence();

    /// GetTokPrecedence - Get the precedence of the pending binary operator token.
    int GetTokPrecedence();

    /// LogError* - These are little helper functions for error handling.
    std::unique_ptr<ExprAST> LogError(const char *Str);
    std::unique_ptr<PrototypeAST> LogErrorP(const char *Str);
    
    std::unique_ptr<ExprAST> ParseNumberExpr();
    std::unique_ptr<ExprAST> ParseParenExpr();
    std::unique_ptr<ExprAST> ParseIdentifierExpr();
    std::unique_ptr<ExprAST> ParseIfExpr();
    std::unique_ptr<ExprAST> ParseForExpr();
    std::unique_ptr<ExprAST> ParseVarExpr();
    std::unique_ptr<ExprAST> ParsePrimary();
    std::unique_ptr<ExprAST> ParseUnary();
    std::unique_ptr<ExprAST> ParseBinOpRHS(int, std::unique_ptr<ExprAST>);
    std::unique_ptr<ExprAST> ParseExpression();
    std::unique_ptr<PrototypeAST> ParsePrototype();
    std::unique_ptr<FunctionAST> ParseDefinition();
    std::unique_ptr<FunctionAST> ParseTopLevelExpr();
    std::unique_ptr<PrototypeAST> ParseExtern();
    
    void HandleExtern();
    void HandleDefinition();
    void HandleTopLevelExpression();
    void parse();
};

#endif
