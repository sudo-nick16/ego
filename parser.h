#include "ast.h"
#include "lexer.h"

#ifndef parser_h
#define parser_h

class Parser {
public:
  Parser() {}
  Parser(std::vector<Token> toks);

  Node *parse_primary();

  Node *parse_expression(int min_prec);

  bool is_next(TokenType type);

  Node *parse_let_statement();

  Node *parse_if_statement();

  Node *parse_return_statement();

  Node *parse_function_statement();

  Node *parse_call_expression();

  Node *parse_while_statement();

  Node *parse_assignment_expression();

  Node *parse_array_expression();

  Node *parse_member_expression();

  std::vector<Node *> parse(TokenType end_token);

  Token peek_token(int i);
  void advance_token();

private:
  std::vector<Token> tokens;
  Token curr_token = Token{TokenType::Eof, "\0"};
  int curr_idx = 0;
  int next_idx = 0;
};

enum Prec {
  Prec0,
  Prec1,
  Prec2,
  Prec3,
  Prec4,
};

enum Assoc { Left, Right };

class OpInfo {
public:
  Prec prec;
  Assoc assoc;
};

extern std::unordered_map<TokenType, OpInfo> OpInfoMap;

class ParseError : public std::exception {
public:
  std::string error_msg;
  ParseError(std::string err);
  const char *what() const noexcept override;
};

#endif // !parser_h
