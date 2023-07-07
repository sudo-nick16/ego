#include "common.h"
#include "tokens.h"

#ifndef lexer_h
#define lexer_h

enum DataType {
  IntType,
  FloatType,
  StringType,
  BoolType,
  ArrayType,
};

class Lexer {
public:
  Lexer(std::string source);
  char peek_char();
  void read_char();
  std::vector<Token> lex();

private:
  std::string source;
  int pos = 0;
  int read_pos = 0;
};

#endif // !lexer_h
