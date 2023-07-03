#include <cctype>
#include <cerrno>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class ParseError : public std::exception {
public:
  std::string error_msg;
  ParseError(std::string err) : error_msg("error while parsing: " + err) {}
  const char *what() const noexcept override { return error_msg.c_str(); }
};

class EvalError : public std::exception {
public:
  std::string error_msg;
  EvalError(std::string err) : error_msg("error while evaluating: " + err) {}
  const char *what() const noexcept override { return error_msg.c_str(); }
};

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
  Minus,
  Div,
  Mul,
  Pow,
  Ident,
  Assign,
  Equal,
  Bang,
  NotEqual,
  Lt,
  Gt,
  Lte,
  Gte,
  Lparen,
  Rparen,
  Lbrace,
  Rbrace,
  Semicolon,
  Comma,
  Function,
  Eof
};

enum NodeType {
  LetNode,
  AssignmentNode,
  ReturnNode,
  IfNode,
  WhileNode,
  FunctionNode,
  BinaryNode,
};

enum DataType {
  IntType,
  FloatType,
  StringType,
  BoolType,
};

class Token {
public:
  TokenType type;
  std::string literal;
  std::string to_string() {
    return "{\n\"type\": \"" + std::to_string(type) + "\",\n\"operator\": \"" +
           literal + "\"\n}";
  }
  friend std::ostream &operator<<(std::ostream &os, const Token &tok) {
    os << "{" << (size_t)tok.type;
    os << ", " << tok.literal << "}";
    return os;
  }
};

bool is_token_type_bool(TokenType t) {
  if (t == True || t == False) {
    return true;
  }
  return false;
}

static std::unordered_map<std::string, TokenType> keywords = {
    {"let", Let},       {"if", If},     {"else", Else},   {"while", While},
    {"return", Return}, {"true", True}, {"false", False}, {"func", Function}};

class Lexer {
public:
  Lexer(std::string source) : source(source) { read_char(); };
  char peek_char() { return source[read_pos]; }
  void read_char() {
    if (read_pos >= source.length()) {
      pos = read_pos;
    } else {
      pos = read_pos;
      read_pos++;
    }
  }
  std::vector<Token> lex() {
    std::vector<Token> tokens;
    while (true) {
      while (isblank(source[pos])) {
        read_char();
      }
      switch (source[pos]) {
      case ';': {
        tokens.push_back(Token{Semicolon, ";"});
        break;
      }
      case ',': {
        tokens.push_back(Token{Comma, ","});
        break;
      }
      case '+': {
        tokens.push_back(Token{Plus, "+"});
        break;
      }
      case '-': {
        tokens.push_back(Token{Minus, "-"});
        break;
      }
      case '/': {
        tokens.push_back(Token{Div, "/"});
        break;
      }
      case '*': {
        tokens.push_back(Token{Mul, "*"});
        break;
      }
      case '^': {
        tokens.push_back(Token{Pow, "^"});
        break;
      }
      case '=': {
        if (peek_char() == '=') {
          tokens.push_back(Token{Equal, "=="});
          read_char();
          break;
        }
        tokens.push_back(Token{Assign, "="});
        break;
      }
      case '!': {
        if (peek_char() == '=') {
          tokens.push_back(Token{NotEqual, "!="});
          read_char();
          break;
        }
        tokens.push_back(Token{Bang, "!"});
        break;
      }
      case '<': {
        if (peek_char() == '=') {
          tokens.push_back(Token{Lte, "<="});
          read_char();
          break;
        }
        tokens.push_back(Token{Lt, "<"});
        break;
      }
      case '>': {
        if (peek_char() == '=') {
          tokens.push_back(Token{Gte, ">="});
          read_char();
          break;
        }
        tokens.push_back(Token{Gt, ">"});
        break;
      }
      case '(': {
        tokens.push_back(Token{Lparen, "("});
        break;
      }
      case ')': {
        tokens.push_back(Token{Rparen, ")"});
        break;
      }
      case '{': {
        tokens.push_back(Token{Lbrace, "{"});
        break;
      }
      case '}': {
        tokens.push_back(Token{Rbrace, "}"});
        break;
      }
      case '"': {
        std::string str;
        read_char();
        while (source[pos] != '"') {
          str += source[pos];
          read_char();
        }
        tokens.push_back(Token{String, str});
        break;
      }
      case '\0': {
        tokens.push_back(Token{Eof, "Eof"});
        return tokens;
      }

      default: {
        if (isdigit(source[pos])) {
          std::string number;
          while (isdigit(source[pos])) {
            number += source[pos];
            read_char();
          }
          tokens.push_back(Token{Number, number});
          continue;
        } else if (isalpha(source[pos])) {
          std::string identifier;
          while (isalpha(source[pos])) {
            identifier += source[pos];
            read_char();
          }
          if (keywords.find(identifier) != keywords.end()) {
            tokens.push_back(Token{
                keywords[identifier],
                identifier,
            });
            continue;
          }
          tokens.push_back(Token{
              Ident,
              identifier,
          });
          continue;
        }
      }
      }
      read_char();
    }
    return tokens;
  };

private:
  std::string source;
  int pos = 0;
  int read_pos = 0;
};

bool is_binary_op(TokenType t) {
  if (t == Plus || t == Minus || t == Div || t == Mul || t == Pow || t == Lt ||
      t == Gt || t == Lte || t == Gte || t == Equal) {
    return true;
  }
  return false;
}

bool assert(TokenType type, TokenType assert_type) {
  if (type != assert_type) {
    throw ParseError("assertion failed");
    return false;
  }
  return true;
}

class Node {
public:
  virtual std::string statement_type() = 0;
  virtual std::string to_string() = 0;
};

template <typename NodeType>
std::string nodes_to_str(std::vector<NodeType *> nodes) {
  std::string s = "[";
  for (int i = 0; i < nodes.size(); i++) {
    s += nodes[i]->to_string();
    if (i != nodes.size() - 1) {
      s += ", ";
    }
  }
  s += "]";
  return s;
}

class Literal : public Node {
public:
  Literal(std::string value, DataType data_type)
      : value(value), data_type(data_type){};
  std::string to_string() {
    return "{\n\"type\": \"" + type + "\",\n\"value\": \"" + value +
           "\",\n\"data_type\": \"" + std::to_string(data_type) + "\"\n}";
  }
  std::string statement_type() { return type; };
  std::string type = "Literal";
  DataType data_type;
  std::string value;
};

class Identifier : public Node {
public:
  Identifier(std::string name) : name(name){};
  std::string to_string() {
    return "{\n\"type\": \"" + type + "\",\n\"name\": \"" + name + "\"\n}";
  }
  std::string statement_type() { return type; };
  std::string type = "Identifier";
  std::string name;
};

class Object {
public:
  virtual DataType type() = 0;
  virtual std::string inspect() = 0;
};

class IntegerObject : public Object {
public:
  IntegerObject(int value) : value(value) {}
  DataType type() { return IntType; }
  std::string inspect() { return std::to_string(value); }
  int value;
};

class FloatObject : public Object {
public:
  FloatObject(float value) : value(value) {}
  DataType type() { return FloatType; }
  std::string inspect() { return std::to_string(value); }
  float value;
};

class StringObject : public Object {
public:
  StringObject(std::string value) : value(value) {}
  DataType type() { return StringType; }
  std::string inspect() { return value; }
  std::string value;
};

class BoolObject : public Object {
public:
  BoolObject(bool value) : value(value) {}
  DataType type() { return BoolType; }
  std::string inspect() { return std::to_string(value); }
  bool value;
};

class Environment {
public:
  Environment() {}
  std::unordered_map<std::string, Object *> store;

  Object *get_identifier(std::string name) {
    if (store.find(name) != store.end()) {
      return store[name];
    }
    return nullptr;
  }

  void set_identifier(std::string name, Object *obj) { store[name] = obj; }
};

class BinaryExpression : public Node {
public:
  BinaryExpression(Node *left, Node *right, Token op)
      : left(left), right(right), op(op){};
  std::string to_string() {
    return "{\n\"type\": \"" + type + "\",\n\"left\": " + left->to_string() +
           ",\n\"right\": " + right->to_string() +
           ",\n\"operator\": " + op.to_string() + "\n}";
  }
  std::string statement_type() { return type; };
  std::string type = "BinaryExpression";
  Node *left;
  Token op;
  Node *right;
};

class LetStatement : public Node {
public:
  LetStatement(Identifier ident, Node *value) : ident(ident), value(value) {}
  std::string statement_type() { return type; };
  std::string to_string() {
    return "{\n\"type\": \"" + type +
           "\",\n\"identifier\": " + ident.to_string() +
           ",\n\"value\": " + value->to_string() + "\n}";
  }
  std::string type = "LetStatement";
  Identifier ident;
  Node *value;
  Environment *env;
};

class IfStatement : public Node {
public:
  IfStatement(Node *condition, std::vector<Node *> consequent,
              std::vector<Node *> alternate)
      : condition(condition), consequent(consequent), alternate(alternate){};
  std::string to_string() {
    return "{\n\"type\": \"" + type +
           "\",\n\"condition\": " + condition->to_string() +
           ",\n\"consequent\": " + nodes_to_str(consequent) +
           ",\n\"alternate\":" + nodes_to_str(alternate) + "\n}";
  };
  std::string statement_type() { return type; };
  std::string type = "IfStatement";
  Node *condition;
  std::vector<Node *> consequent;
  std::vector<Node *> alternate;
};

class ReturnStatement : public Node {
public:
  ReturnStatement(Node *value) : value(value){};
  std::string to_string() {
    return "{\n\"type\": \"" + type + "\",\n\"value\": " + value->to_string() +
           "\n}";
  };
  std::string statement_type() { return type; };
  std::string type = "ReturnStatement";
  Node *value;
};

class FunctionStatement : public Node {
public:
  FunctionStatement(Identifier ident, std::vector<Identifier *> params,
                    std::vector<Node *> block)
      : params(params), ident(ident), block(block){};
  std::string to_string() {
    return "{\n\"type\": \"" + type + "\",\n\"ident\": " + ident.to_string() +
           ",\n\"params\": " + nodes_to_str(params) +
           ",\n\"block\": " + nodes_to_str(block) + "\n}";
  };
  std::string statement_type() { return type; };
  std::string type = "FunctionStatement";
  std::vector<Identifier *> params;
  Identifier ident;
  std::vector<Node *> block;
};

class CallExpression : public Node {
public:
  CallExpression(Identifier callee, std::vector<Node *> args)
      : callee(callee), args(args){};
  std::string to_string() {
    return "{\n\"type\": \"" + type + "\",\n\"callee\": " + callee.to_string() +
           ",\n\"args\": " + nodes_to_str(args) + "\n}";
  };
  std::string statement_type() { return type; };
  std::string type = "CallExpression";
  Identifier callee;
  std::vector<Node *> args;
};

class WhileStatement : public Node {
public:
  WhileStatement(Node *condition, std::vector<Node *> block)
      : condition(condition), block(block){};
  std::string to_string() {
    return "{\n\"type\": \"" + type +
           "\",\n\"condition\": " + condition->to_string() +
           ",\n\"block\": " + nodes_to_str(block) + "\n}";
  };
  std::string statement_type() { return type; };
  std::string type = "WhileStatement";
  Node *condition;
  std::vector<Node *> block;
};

class AssignmentExpression : public Node {
public:
  AssignmentExpression(Identifier ident, Node *value)
      : ident(ident), value(value) {}
  std::string statement_type() { return type; };
  std::string to_string() {
    return "{\n\"type\": \"" + type +
           "\",\n\"identifier\": " + ident.to_string() +
           ",\n\"value\": " + value->to_string() + "\n}";
  }
  std::string type = "AssignmentExpression";
  Identifier ident;
  Node *value;
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

static std::unordered_map<TokenType, OpInfo> OpInfoMap{
    {Lt, OpInfo{Prec0, Left}},   {Lte, OpInfo{Prec0, Left}},
    {Gt, OpInfo{Prec0, Left}},   {Gte, OpInfo{Prec0, Left}},
    {Plus, OpInfo{Prec1, Left}}, {Minus, OpInfo{Prec1, Left}},
    {Mul, OpInfo{Prec2, Left}},  {Div, OpInfo{Prec2, Left}},
    {Pow, OpInfo{Prec3, Right}},
};

class Parser {
public:
  Parser(std::vector<Token> *tokens) : tokens(tokens) { advance_token(); };

  Node *parse_primary() {
    // std::cout << "parse_primary = " << *curr_token << "\n";
    if (curr_token->type == Lparen) {
      advance_token();
      Node *node = parse_expression(Prec0);
      if (curr_token->type != Rparen) {
        throw ParseError("expected ) while parsing expression");
      }
      advance_token();
      return node;
    } else if (is_binary_op(curr_token->type)) {
      throw ParseError("unexpected op");
    } else if (curr_token->type == Eof) {
      throw ParseError("expression ended unexpectedly");
    } else {
      // std::cout << "parse_primary (got num/ident) = " << *curr_token <<
      // "\n";
      if (curr_token->type == Number) {
        Node *node = new Literal(curr_token->literal, IntType);
        advance_token();
        return node;
      };
      if (curr_token->type == Ident) {
        Node *node = new Identifier(curr_token->literal);
        advance_token();
        return node;
      }
      if (curr_token->type == String) {
        Node *node = new Literal(curr_token->literal, StringType);
        advance_token();
        return node;
      }
      if (is_token_type_bool(curr_token->type)) {
        Node *node = new Literal(curr_token->literal, BoolType);
        advance_token();
        return node;
      }
      throw ParseError(
          "invalid token, expected a literal or identifier but got " +
          curr_token->to_string());
    }
  }

  Node *parse_expression(int min_prec) {
    Node *left = parse_primary();
    while (true) {
      if (curr_token->type == Eof || !is_binary_op(curr_token->type) ||
          OpInfoMap[curr_token->type].prec < min_prec) {
        break;
      }
      if (!is_binary_op(curr_token->type)) {
        throw ParseError("no binary operator provided.");
      }
      Token curr_op = *curr_token;
      OpInfo curr_op_info = OpInfoMap[curr_token->type];
      int next_min_prec = curr_op_info.assoc == Left ? curr_op_info.prec + 1
                                                     : curr_op_info.prec;
      advance_token();
      Node *right = parse_expression(next_min_prec);
      // std::cout << "right = " << right->to_string() << "\n";
      left = new BinaryExpression(left, right, curr_op);
    }
    return left;
  }

  bool is_next(TokenType type) {
    if (peek_token(1).type == type) {
      return true;
    }
    return false;
  }

  Node *parse_let_statement() {
    if (!is_next(Ident)) {
      throw ParseError("expected identifier");
    }
    advance_token();
    std::cout << "curr_token = " << *curr_token << "\n";
    Identifier ident = Identifier(curr_token->literal);
    if (!is_next(Assign)) {
      throw ParseError("expected assignment operator");
    }
    advance_token();
    advance_token();
    Node *value = parse_expression(Prec0);
    return new LetStatement(ident, value);
  };

  Node *parse_if_statement() {
    if (!is_next(Lparen)) {
      throw ParseError("expected (");
    }
    advance_token();
    std::cout << "if statement curr_token = " << *curr_token << "\n";
    advance_token();
    Node *condition = parse_expression(Prec0);
    if (curr_token->type != Rparen) {
      throw ParseError("expected )");
    }
    if (!is_next(Lbrace)) {
      throw ParseError("expected {");
    }
    advance_token();
    std::vector<Node *> consequent = parse(Rbrace);
    if (curr_token->type != Rbrace) {
      throw ParseError("expected } in if block");
    }
    std::cout << "after consequent = " << *curr_token << "\n";
    std::vector<Node *> alternate;
    if (is_next(Else)) {
      advance_token();
      if (!is_next(Lbrace)) {
        throw ParseError("expected {");
      }
      advance_token();
      alternate = parse(Rbrace);
      if (curr_token->type != Rbrace) {
        throw ParseError("expected } in else block");
      }
    }
    advance_token();
    IfStatement *if_statement =
        new IfStatement(condition, consequent, alternate);
    return if_statement;
  }

  Node *parse_return_statement() {
    advance_token();
    Node *value = parse_expression(Prec0);
    ReturnStatement *return_statement = new ReturnStatement(value);
    return return_statement;
  }

  Node *parse_function_statement() {
    if (!is_next(Ident)) {
      throw ParseError("expected identifier");
    }
    advance_token();
    Identifier ident = Identifier(curr_token->literal);
    if (!is_next(Lparen)) {
      throw ParseError("expected (");
    }
    advance_token();
    std::vector<Identifier *> params = {};
    while (curr_token->type != Rparen) {
      if (!is_next(Ident)) {
        throw ParseError("expected identifier");
      }
      advance_token();
      params.push_back(new Identifier(curr_token->literal));
      if (!is_next(Comma) && !is_next(Rparen)) {
        throw ParseError("expected , or )");
      }
      advance_token();
    }
    if (!is_next(Lbrace)) {
      throw ParseError("expected {");
    }
    std::vector<Node *> block = parse(Rbrace);
    if (curr_token->type != Rbrace) {
      throw ParseError("expected }");
    }
    advance_token();
    FunctionStatement *function_statement =
        new FunctionStatement(ident, params, block);

    return function_statement;
  }

  Node *parse_call_expression() {
    Identifier callee = Identifier(curr_token->literal);
    advance_token();
    advance_token();
    std::cout << "curr_token call exprssion = " << *curr_token << "\n";
    std::vector<Node *> args = {};
    while (curr_token->type != Rparen) {
      Node *arg = parse_expression(Prec0);
      // std::cout << "arg = " << arg->to_string() << "\n";
      args.push_back(arg);
      if (curr_token->type == Comma) {
        advance_token();
      } else if (curr_token->type == Rparen) {
        break;
      } else {
        throw ParseError("expected , or )");
      }
    }
    CallExpression *call_expression = new CallExpression(callee, args);
    return call_expression;
  }

  Node *parse_while_statement() {
    if (!is_next(Lparen)) {
      throw ParseError("expected (");
    }
    advance_token();
    std::cout << "if statement curr_token = " << *curr_token << "\n";
    advance_token();
    Node *condition = parse_expression(Prec0);
    if (curr_token->type != Rparen) {
      throw ParseError("expected )");
    }
    if (!is_next(Lbrace)) {
      throw ParseError("expected {");
    }
    advance_token();
    std::vector<Node *> block = parse(Rbrace);
    if (curr_token->type != Rbrace) {
      throw ParseError("expected } in while block");
    }
    advance_token();
    WhileStatement *while_statement = new WhileStatement(condition, block);
    return while_statement;
  }

  Node *parse_assignment_expression() {
    Identifier ident = Identifier(curr_token->literal);
    advance_token();
    advance_token();
    Node *value = parse_expression(Prec0);
    return new AssignmentExpression(ident, value);
  }

  std::vector<Node *> parse(TokenType end_token) {
    std::vector<Node *> program = {};
    while (curr_token != nullptr && curr_token->type != end_token) {
      switch (curr_token->type) {
      case Lparen: {
        Node *node = parse_expression(Prec0);
        program.push_back(node);
        break;
      }
      case Let: {
        std::cout << "let statement\n";
        Node *node = parse_let_statement();
        program.push_back(node);
        std::cout << "let statement done\n";
        continue;
        break;
      }
      case If: {
        std::cout << "if statement\n";
        Node *node = parse_if_statement();
        program.push_back(node);
        continue;
        break;
      }
      case While: {
        std::cout << "while statement\n";
        Node *node = parse_while_statement();
        program.push_back(node);
        continue;
        break;
      }
      case Number: {
        Node *node = parse_expression(Prec0);
        program.push_back(node);
        break;
      }
      case Function: {
        std::cout << "function statement\n";
        Node *node = parse_function_statement();
        program.push_back(node);
        std::cout << "function statement done\n";
        continue;
        break;
      }
      case Return: {
        Node *node = parse_return_statement();
        program.push_back(node);
        continue;
        break;
      }
      case Eof: {
        Node *node = nullptr;
        program.push_back(node);
        return program;
      }
      default: {
        if (curr_token->type == Ident) {
          if (is_next(Lparen)) {
            Node *node = parse_call_expression();
            program.push_back(node);
            continue;
          } else if (is_next(Assign)) {
            Node *node = parse_assignment_expression();
            program.push_back(node);
            continue;
          }
        }
      }
      }
      advance_token();
    }
    return program;
  };

  Token peek_token(int i) { return (*tokens)[curr_idx + i]; };
  void advance_token() {
    if (next_idx >= tokens->size()) {
      curr_idx = next_idx;
      curr_token = nullptr;
    } else {
      curr_idx = next_idx;
      curr_token = &(*tokens)[curr_idx];
      next_idx++;
    }
  }

private:
  std::vector<Token> *tokens;
  Token *curr_token;
  int curr_idx;
  int next_idx;
};

Object *get_obj_from_literal(Literal *l) {
  switch (l->data_type) {
  case IntType: {
    return new IntegerObject(stoi(l->value));
  }
  case BoolType: {
    return new BoolObject(l->value == "true");
  }
  case FloatType: {
    return new FloatObject(stof(l->value));
  }
  case StringType: {
    return new StringObject(l->value);
  }
  default: {
    std::cout << "literal : " << l->to_string() << "\n";
    throw EvalError("unknown literal type" + l->to_string());
  }
  }
  return new StringObject(l->value);
}

Object *evaluate_operator(Object *left, Object *right, Token op) {
  if (left->type() != right->type()) {
    throw EvalError("type mismatch");
  }
  if (left->type() == BoolType || right->type() == BoolType) {
    throw EvalError("cannot use bool with operator");
  }
  switch (op.type) {
  case Plus: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new IntegerObject(left_int->value + right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new FloatObject(left_float->value + right_float->value);
    } else if (left->type() == StringType) {
      StringObject *left_string = (StringObject *)left;
      StringObject *right_string = (StringObject *)right;
      return new StringObject(left_string->value + right_string->value);
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case Minus: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new IntegerObject(left_int->value - right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new FloatObject(left_float->value - right_float->value);
    } else if (left->type() == StringType) {
      throw EvalError("invalid operation on string");
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case Mul: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new IntegerObject(left_int->value * right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new FloatObject(left_float->value * right_float->value);
    } else if (left->type() == StringType) {
      throw EvalError("invalid operation on string");
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case Div: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new IntegerObject(left_int->value / right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new FloatObject(left_float->value / right_float->value);
    } else if (left->type() == StringType) {
      throw EvalError("invalid operation on string");
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case Equal: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new BoolObject(left_int->value == right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new BoolObject(left_float->value == right_float->value);
    } else if (left->type() == StringType) {
      throw EvalError("invalid operation on string");
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case NotEqual: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new BoolObject(left_int->value != right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new BoolObject(left_float->value != right_float->value);
    } else if (left->type() == StringType) {
      throw EvalError("invalid operation on string");
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case Lt: {
    if (left->type() == IntType) {
      IntegerObject *left_int = (IntegerObject *)left;
      IntegerObject *right_int = (IntegerObject *)right;
      return new BoolObject(left_int->value < right_int->value);
    } else if (left->type() == FloatType) {
      FloatObject *left_float = (FloatObject *)left;
      FloatObject *right_float = (FloatObject *)right;
      return new BoolObject(left_float->value < right_float->value);
    } else if (left->type() == StringType) {
      throw EvalError("invalid operation on string");
    } else {
      throw EvalError("type mismatch");
    }
    break;
  }
  case Gt: {
    // TODO: find a short way to do this, maybe use define?
    break;
  }
  default:
    throw EvalError("invalid expression");
  }
  return nullptr;
}

Object *evaluate_expression(Node *node, Environment *env) {
  if (node->statement_type() == "BinaryExpression") {
    BinaryExpression *bNode = (BinaryExpression *)node;
    Object *left = evaluate_expression(bNode->left, env);
    Object *right = evaluate_expression(bNode->right, env);
    return evaluate_operator(left, right, bNode->op);
  } else if (node->statement_type() == "Literal") {
    return get_obj_from_literal((Literal *)node);
  } else if (node->statement_type() == "Identifier") {
    Object *obj = env->get_identifier(((Identifier *)node)->name);
    if (obj == nullptr) {
      throw EvalError("undefined identifier");
    }
    return obj;
  }
  throw EvalError("invalid initialization value");
  return nullptr;
}

class Evaluator {
public:
  Evaluator(std::vector<Node *> &program) : program(program) {
    global_env = new Environment();
  }

  void evaluate(Environment *env) {
    for (auto node : program) {
      std::string type = node->statement_type();
      if (type == "LetStatement") {
        LetStatement *letNode = (LetStatement *)node;
        std::string name = letNode->ident.name;
        if (env->store.find(name) != env->store.end()) {
          throw EvalError("variable already defined");
        }
        Object *obj = evaluate_expression(letNode->value, env);
        env->store[name] = obj;
      } else if (type == "AssignmentExpression") {
        AssignmentExpression *assNode = (AssignmentExpression *)node;
        std::string name = assNode->ident.name;
        if (env->store.find(name) == env->store.end()) {
          throw EvalError("variable not defined");
        }
        Object *obj = evaluate_expression(assNode->value, env);
        env->store[name] = obj;
      } else if (type == "IfStatement") {
      }
    }

    std::cout << "-----\n";
    for (auto ele : env->store) {
      std::cout << ele.first << " -> " << ele.second->inspect() << "\n";
    }
  }
  Environment *global_env;
  std::vector<Node *> program;
};

int main() {
  std::ifstream file("test/test.ego");
  if (file.is_open()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.lex();
    for (auto token : tokens) {
      std::cout << token << std::endl;
    }
    Parser parser(&tokens);

    std::vector<Node *> program = parser.parse(Eof);

    Evaluator evaluator = Evaluator(program);
    Environment *global_env = new Environment();

    std::cout << nodes_to_str(program) << std::endl;
    evaluator.evaluate(global_env);

    std::ofstream out("test/test_ast.json");
    if (!out.is_open()) {
      std::cout << "Unable to open json file" << std::endl;
    }
    out << nodes_to_str(program);
    out.close();

    // for (auto node : program) {
    //   std::cout << node->to_string() << std::endl;
    // }
  } else {
    std::cout << "Unable to open file" << std::endl;
  }
  return 0;
}
