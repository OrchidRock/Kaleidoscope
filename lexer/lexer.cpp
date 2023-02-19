#include "../include/lexer.h"
using namespace Token;

LexerFile::LexerFile(llvm::SourceMgr& SrcMgr) : SrcMgr(SrcMgr) {
    CurBuffer = SrcMgr.getMainFileID();
    CurBuf = SrcMgr.getMemoryBuffer(CurBuffer)->getBuffer();
    CurPtr = CurBuf.begin();
}

namespace charinfo {
LLVM_READNONE inline bool isASCII(char Ch) {
  return static_cast<unsigned char>(Ch) <= 127;
}

LLVM_READNONE inline bool isVerticalWhitespace(char Ch) {
  return isASCII(Ch) && (Ch == '\r' || Ch == '\n');
}

LLVM_READNONE inline bool isHorizontalWhitespace(char Ch) {
  return isASCII(Ch) && (Ch == ' ' || Ch == '\t' ||
                         Ch == '\f' || Ch == '\v');
}

LLVM_READNONE inline bool isWhitespace(char Ch) {
  return isHorizontalWhitespace(Ch) ||
         isVerticalWhitespace(Ch);
}

LLVM_READNONE inline bool isDigit(char Ch) {
  return isASCII(Ch) && Ch >= '0' && Ch <= '9';
}

LLVM_READNONE inline bool isHexDigit(char Ch) {
  return isASCII(Ch) &&
         (isDigit(Ch) || (Ch >= 'A' && Ch <= 'F'));
}

LLVM_READNONE inline bool isIdentifierHead(char Ch) {
  return isASCII(Ch) &&
         (Ch == '_' || (Ch >= 'A' && Ch <= 'Z') ||
          (Ch >= 'a' && Ch <= 'z'));
}

LLVM_READNONE inline bool isIdentifierBody(char Ch) {
  return isIdentifierHead(Ch) || isDigit(Ch);
}
} // namespace charinfo

int LexerFile::gettok() {
   
  //static int LastChar = ' ';

  // Skip any whitespace.
  while (*CurPtr && charinfo::isWhitespace(*CurPtr))
    CurPtr ++;
  

  if (charinfo::isIdentifierHead(*CurPtr)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = *CurPtr;
    CurPtr++;
    while (charinfo::isIdentifierBody(*CurPtr)){
      IdentifierStr += *CurPtr;
      CurPtr++;
    }

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    if (IdentifierStr == "if")
      return tok_if;
    if (IdentifierStr == "then")
      return tok_then;
    if (IdentifierStr == "else")
      return tok_else;
    if (IdentifierStr == "for")
      return tok_for;
    if (IdentifierStr == "in")
      return tok_in;
    if (IdentifierStr == "binary")
      return tok_binary;
    if (IdentifierStr == "unary")
      return tok_unary;
    if (IdentifierStr == "var")
      return tok_var;
    return tok_identifier;
  }

  if (charinfo::isDigit(*CurPtr) || *CurPtr == '.') { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += *CurPtr++;
    } while (charinfo::isDigit(*CurPtr) || *CurPtr == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  if (*CurPtr == '#') {
    // Comment until end of line.
    do
      CurPtr ++;
    while (*CurPtr != EOF && *CurPtr != '\n' && *CurPtr != '\r');

    if (*CurPtr != EOF)
      return gettok();
  }

  // Check for end of file.  Don't eat the EOF.
  if (!*CurPtr)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = *CurPtr++;
  return ThisChar;
}

int LexerSimple::gettok() {
  static int LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = getchar();

  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getchar())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    if (IdentifierStr == "if")
      return tok_if;
    if (IdentifierStr == "then")
      return tok_then;
    if (IdentifierStr == "else")
      return tok_else;
    if (IdentifierStr == "for")
      return tok_for;
    if (IdentifierStr == "in")
      return tok_in;
    if (IdentifierStr == "binary")
      return tok_binary;
    if (IdentifierStr == "unary")
      return tok_unary;
    if (IdentifierStr == "var")
      return tok_var;
    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  if (LastChar == '#') {
    // Comment until end of line.
    do
      LastChar = getchar();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

