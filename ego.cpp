#include <cctype>
#include <cerrno>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class ParserError : public std::exception {
public:
  std::string error_msg;
  ParserError(std::string err) : error_msg(err) {}
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

enum DataType {
  IntType,
  FloatType,
  StringType,
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

bool is_digit(char c) {
  if (c >= '0' && c <= '9') {
    return true;
  } else {
    return false;
  }
}

bool is_alpha(char c) {
  if (c >= 'a' && c <= 'z') {
    return true;
  } else if (c >= 'A' && c <= 'Z') {
    return true;
  } else {
    return false;
  }
}

bool is_blank(char c) {
  if (c == ' ' || c == '\t' || c == '\n') {
    return true;
  } else {
    return false;
  }
}

static std::map<std::string, TokenType> keywords = {
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
    throw ParserError("assertion failed");
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

static std::map<TokenType, OpInfo> OpInfoMap{
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
        throw ParserError("expected ) while parsing expression");
      }
      advance_token();
      return node;
    } else if (is_binary_op(curr_token->type)) {
      throw ParserError("unexpected op");
    } else if (curr_token->type == Eof) {
      throw ParserError("expression ended unexpectedly");
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
      throw ParserError(
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
        throw ParserError("no binary operator provided.");
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
      throw ParserError("expected identifier");
    }
    advance_token();
    std::cout << "curr_token = " << *curr_token << "\n";
    Identifier ident = Identifier(curr_token->literal);
    if (!is_next(Assign)) {
      throw ParserError("expected assignment operator");
    }
    advance_token();
    advance_token();
    Node *value = parse_expression(Prec0);
    return new LetStatement(ident, value);
  };

  Node *parse_if_statement() {
    if (!is_next(Lparen)) {
      throw ParserError("expected (");
    }
    advance_token();
    std::cout << "if statement curr_token = " << *curr_token << "\n";
    advance_token();
    Node *condition = parse_expression(Prec0);
    if (curr_token->type != Rparen) {
      throw ParserError("expected )");
    }
    if (!is_next(Lbrace)) {
      throw ParserError("expected {");
    }
    advance_token();
    std::vector<Node *> consequent = parse(Rbrace);
    if (curr_token->type != Rbrace) {
      throw ParserError("expected } in if block");
    }
    std::cout << "after consequent = " << *curr_token << "\n";
    std::vector<Node *> alternate;
    if (is_next(Else)) {
      advance_token();
      if (!is_next(Lbrace)) {
        throw ParserError("expected {");
      }
      advance_token();
      alternate = parse(Rbrace);
      if (curr_token->type != Rbrace) {
        throw ParserError("expected } in else block");
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
      throw ParserError("expected identifier");
    }
    advance_token();
    Identifier ident = Identifier(curr_token->literal);
    if (!is_next(Lparen)) {
      throw ParserError("expected (");
    }
    advance_token();
    std::vector<Identifier *> params = {};
    while (curr_token->type != Rparen) {
      if (!is_next(Ident)) {
        throw ParserError("expected identifier");
      }
      advance_token();
      params.push_back(new Identifier(curr_token->literal));
      if (!is_next(Comma) && !is_next(Rparen)) {
        throw ParserError("expected , or )");
      }
      advance_token();
    }
    if (!is_next(Lbrace)) {
      throw ParserError("expected {");
    }
    std::vector<Node *> block = parse(Rbrace);
    if (curr_token->type != Rbrace) {
      throw ParserError("expected }");
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
        throw ParserError("expected , or )");
      }
    }
    CallExpression *call_expression = new CallExpression(callee, args);
    return call_expression;
  }

  Node *parse_while_statement() {
    if (!is_next(Lparen)) {
      throw ParserError("expected (");
    }
    advance_token();
    std::cout << "if statement curr_token = " << *curr_token << "\n";
    advance_token();
    Node *condition = parse_expression(Prec0);
    if (curr_token->type != Rparen) {
      throw ParserError("expected )");
    }
    if (!is_next(Lbrace)) {
      throw ParserError("expected {");
    }
    advance_token();
    std::vector<Node *> block = parse(Rbrace);
    if (curr_token->type != Rbrace) {
      throw ParserError("expected } in while block");
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

class Environment {
public:
  Environment() {}
  std::unordered_map<std::string *, std::string> store;
  std::unordered_map<Identifier *, FunctionStatement *> functions;
};

class Evalutor {
public:
  Evalutor(std::vector<Node *> program) : program(program) {
    global_env = new Environment();
  }
  void evaluate(Environment *env) {
    for (auto node : program) {
      std::string type = node->statement_type();
      if (type == "LetStatement") {
        LetStatement *letNode = (LetStatement *)node;
        env->store.find();
      }
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

    std::cout << nodes_to_str(program) << std::endl;

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
