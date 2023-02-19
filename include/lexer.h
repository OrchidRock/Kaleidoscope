#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"

namespace Token {
enum TokenID {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  // control
  tok_if = -6,
  tok_then = -7,
  tok_else = -8,
  tok_for = -9,
  tok_in = -10,

  // operators
  tok_binary = -11,
  tok_unary = -12,

  // var definition
  tok_var = -13
};
    
}

class Lexer {
public:
    std::string IdentifierStr = ""; // Filled in if tok_identifier
    double NumVal = 0;             // Filled in if tok_number
    virtual ~Lexer() {};  
    virtual int gettok() = 0;
};

class LexerFile : public Lexer {
  
  llvm::SourceMgr& SrcMgr;
  const char *CurPtr;
  llvm::StringRef CurBuf;

  /// CurBuffer - This is the current buffer index we're
  /// lexing from as managed by the SourceMgr object.
  unsigned CurBuffer = 0;

public:
    LexerFile(llvm::SourceMgr& SrcMgr);

    int gettok() override;
};

class LexerSimple : public Lexer {
public:
    LexerSimple(){}
    int gettok() override;
};

#endif
