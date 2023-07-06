#include <string>
#include <unordered_map>
#ifndef tokens_h
#define tokens_h

enum TokenType {
  Number,
  True,
  False,
  Return,
  Let,
  If,
  Else,
  While,
  String,
  Plus,
  PlusEq,
  Minus,
  MinusEq,
  Div,
  DivEq,
  Mul,
  MulEq,
  Mod,
  Pow,
  PowEq,
  Ident,
  Assign,
  Equal,
  Bang,
  NotEqual,
  Lt,
  Gt,
  Lte,
  Gte,
  And,
  Or,
  Lparen,
  Rparen,
  Lbrace,
  Rbrace,
  Semicolon,
  Comma,
  Function,
  Eof
};

class Token {
public:
  Token(TokenType t, std::string lit);
  TokenType type;
  std::string literal;
  std::string to_string();
  friend std::ostream &operator<<(std::ostream &os, const Token &tok);
};

extern const std::unordered_map<std::string, TokenType> Keywords;

#endif // !tokens_h
