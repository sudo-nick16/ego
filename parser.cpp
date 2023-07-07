#include "parser.h"
#include "utils.h"

ParseError::ParseError(std::string err)
    : error_msg("error while parsing: " + err) {}
const char *ParseError::what() const noexcept { return error_msg.c_str(); }

std::unordered_map<TokenType, OpInfo> OpInfoMap = {
    {Or, OpInfo{Prec0, Left}},       {And, OpInfo{Prec0, Left}},
    {Lt, OpInfo{Prec1, Left}},       {Lte, OpInfo{Prec1, Left}},
    {Gt, OpInfo{Prec1, Left}},       {Gte, OpInfo{Prec1, Left}},
    {NotEqual, OpInfo{Prec1, Left}}, {Equal, OpInfo{Prec1, Left}},
    {Plus, OpInfo{Prec2, Left}},     {Minus, OpInfo{Prec2, Left}},
    {Mul, OpInfo{Prec3, Left}},      {Div, OpInfo{Prec3, Left}},
    {Mod, OpInfo{Prec4, Left}},      {Pow, OpInfo{Prec4, Right}},
};

Parser::Parser(std::vector<Token> toks) {
  this->tokens = toks;
  advance_token();
};

Node *Parser::parse_primary() {
  if (curr_token.type == Lparen) {
    advance_token();
    Node *node = parse_expression(Prec0);
    if (curr_token.type != Rparen) {
      throw ParseError("expected ) while parsing expression");
    }
    advance_token();
    return node;
  } else if (is_binary_op(curr_token.type)) {
    throw ParseError("unexpected op");
  } else if (curr_token.type == Eof) {
    throw ParseError("expression ended unexpectedly");
  } else {
    if (curr_token.type == Number) {
      if (curr_token.literal.find('.') != std::string::npos) {
        Node *node = new Literal(curr_token.literal, FloatType);
        advance_token();
        return node;
      }
      Node *node = new Literal(curr_token.literal, IntType);
      advance_token();
      return node;
    };
    if (curr_token.type == Ident) {
      if (is_next(Lparen)) {
        return parse_call_expression();
      }
      if (is_next(Lbracket)) {
        return parse_member_expression();
      }
      Node *node = new Identifier(curr_token.literal);
      advance_token();
      return node;
    }
    if (curr_token.type == String) {
      Node *node = new Literal(curr_token.literal, StringType);
      advance_token();
      return node;
    }
    if (is_token_type_bool(curr_token.type)) {
      Node *node = new Literal(curr_token.literal, BoolType);
      advance_token();
      return node;
    }
    if (curr_token.type == Lbracket) {
      return parse_array_expression();
    }
    throw ParseError(
        "invalid token, expected a literal or identifier but got " +
        curr_token.to_string());
  }
}

Node *Parser::parse_expression(int min_prec) {
  Node *left = parse_primary();
  while (true) {
    if (curr_token.type == Eof || !is_binary_op(curr_token.type) ||
        OpInfoMap[curr_token.type].prec < min_prec) {
      break;
    }
    if (!is_binary_op(curr_token.type)) {
      throw ParseError("no binary operator provided.");
    }
    Token curr_op = curr_token;
    OpInfo curr_op_info = OpInfoMap[curr_token.type];
    int next_min_prec =
        curr_op_info.assoc == Left ? curr_op_info.prec + 1 : curr_op_info.prec;
    advance_token();
    Node *right = parse_expression(next_min_prec);
    // std::cout << "right = " << right->to_string() << "\n";
    left = new BinaryExpression(left, right, curr_op);
  }
  return left;
}

bool Parser::is_next(TokenType type) {
  if (peek_token(1).type == type) {
    return true;
  }
  return false;
}

Node *Parser::parse_let_statement() {
  if (!is_next(Ident)) {
    throw ParseError("expected identifier");
  }
  advance_token();
  // std::cout << "curr_token = " << *curr_token << "\n";
  Identifier ident = Identifier(curr_token.literal);
  if (!is_next(Assign)) {
    throw ParseError("expected assignment operator");
  }
  advance_token();
  advance_token();
  if (curr_token.type == Lbracket) {
    Node *value = parse_array_expression();
    return new LetStatement(ident, value);
  }
  Node *value = parse_expression(Prec0);
  return new LetStatement(ident, value);
};

Node *Parser::parse_if_statement() {
  if (!is_next(Lparen)) {
    throw ParseError("expected ( while parsing if condition");
  }
  advance_token();
  // std::cout << "if statement curr_token = " << *curr_token << "\n";
  advance_token();
  Node *condition = parse_expression(Prec0);
  if (curr_token.type != Rparen) {
    throw ParseError("expected ) while parsing if condition");
  }
  if (!is_next(Lbrace)) {
    throw ParseError("expected { while parsing if condition");
  }
  advance_token();
  std::vector<Node *> consequent = parse(Rbrace);
  if (curr_token.type != Rbrace) {
    throw ParseError("expected } in if block");
  }
  // std::cout << "after consequent = " << *curr_token << "\n";
  std::vector<Node *> alternate;
  if (is_next(Else)) {
    advance_token();
    if (!is_next(Lbrace)) {
      throw ParseError("expected {");
    }
    advance_token();
    alternate = parse(Rbrace);
    if (curr_token.type != Rbrace) {
      throw ParseError("expected } in else block");
    }
  }
  advance_token();
  IfStatement *if_statement = new IfStatement(condition, consequent, alternate);
  return if_statement;
}

Node *Parser::parse_return_statement() {
  advance_token();
  Node *value = parse_expression(Prec0);
  ReturnStatement *return_statement = new ReturnStatement(value);
  return return_statement;
}

Node *Parser::parse_function_statement() {
  if (!is_next(Ident)) {
    throw ParseError("missing function name");
  }
  advance_token();
  Identifier ident = Identifier(curr_token.literal);
  if (!is_next(Lparen)) {
    throw ParseError("expected (");
  }
  advance_token();
  // std::cout << "curr_token in function satement after Lparen ( = "
  //           << *curr_token << "\n";
  std::vector<Identifier *> params = {};
  if (is_next(Rparen)) {
    advance_token();
  }
  while (curr_token.type != Rparen) {
    if (!is_next(Ident)) {
      throw ParseError("expected identifier");
    }
    advance_token();
    params.push_back(new Identifier(curr_token.literal));
    if (!is_next(Comma) && !is_next(Rparen)) {
      throw ParseError("expected , or ) in function " + ident.name);
    }
    advance_token();
  }
  if (!is_next(Lbrace)) {
    throw ParseError("expected {");
  }
  std::vector<Node *> block = parse(Rbrace);
  if (curr_token.type != Rbrace) {
    throw ParseError("expected }");
  }
  advance_token();
  FunctionStatement *function_statement =
      new FunctionStatement(ident, params, block);

  return function_statement;
}

Node *Parser::parse_call_expression() {
  Identifier callee = Identifier(curr_token.literal);
  advance_token();
  advance_token();
  std::vector<Node *> args = {};
  while (curr_token.type != Rparen) {
    Node *arg = parse_expression(Prec0);
    args.push_back(arg);
    if (curr_token.type == Comma) {
      advance_token();
    } else if (curr_token.type == Rparen) {
      break;
    } else {
      // std::cout << "curr_token = " << curr_token << "\n";
      throw ParseError("expected , or ) in function call " + callee.name);
    }
  }
  advance_token();
  CallExpression *call_expression = new CallExpression(callee, args);
  return call_expression;
}

Node *Parser::parse_while_statement() {
  if (!is_next(Lparen)) {
    throw ParseError("expected (");
  }
  advance_token();
  advance_token();
  Node *condition = parse_expression(Prec0);
  if (curr_token.type != Rparen) {
    throw ParseError("expected )");
  }
  // std::cout << "while statement curr_token = " << curr_token << "\n";
  if (!is_next(Lbrace)) {
    throw ParseError("expected { in while block");
  }
  advance_token();
  std::vector<Node *> block = parse(Rbrace);
  if (curr_token.type != Rbrace) {
    throw ParseError("expected } in while block");
  }
  advance_token();
  WhileStatement *while_statement = new WhileStatement(condition, block);
  return while_statement;
}

Node *Parser::parse_assignment_expression() {
  Identifier ident = Identifier(curr_token.literal);
  advance_token();
  advance_token();
  if (curr_token.type == Lbracket) {
    Node *value = parse_array_expression();
    return new AssignmentExpression(ident, value);
  }
  Node *value = parse_expression(Prec0);
  return new AssignmentExpression(ident, value);
}

std::vector<Node *> Parser::parse(TokenType end_token) {
  std::vector<Node *> program = {};
  while (curr_token.type != end_token) {
    switch (curr_token.type) {
    case Lparen: {
      Node *node = parse_expression(Prec0);
      program.push_back(node);
      break;
    }
    case Let: {
      // std::cout << "let statement\n";
      Node *node = parse_let_statement();
      program.push_back(node);
      // std::cout << "let statement done\n";
      continue;
      break;
    }
    case If: {
      // std::cout << "if statement\n";
      Node *node = parse_if_statement();
      program.push_back(node);
      continue;
      break;
    }
    case While: {
      // std::cout << "while statement\n";
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
      // std::cout << "function statement\n";
      Node *node = parse_function_statement();
      program.push_back(node);
      // std::cout << "function statement done\n";
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
      if (curr_token.type == Ident) {
        if (is_next(Lparen)) {
          Node *node = parse_call_expression();
          program.push_back(node);
          continue;
        } else if (is_next(Assign)) {
          Node *node = parse_assignment_expression();
          program.push_back(node);
          continue;
        } else if (is_next(Lbracket)) {
          Node *node = parse_member_expression();
          program.push_back(node);
        }
      }
    }
    }
    advance_token();
  }
  return program;
};

Token Parser::peek_token(int i) { return tokens[curr_idx + i]; };
void Parser::advance_token() {
  if (next_idx >= tokens.size()) {
    curr_idx = next_idx;
    curr_token = Token{Eof, "Eof"};
  } else {
    curr_idx = next_idx;
    curr_token = tokens[curr_idx];
    next_idx++;
  }
}

Node *Parser::parse_array_expression() {
  std::vector<Node *> elements = {};
  advance_token();
  while (curr_token.type != Rbracket) {
    Node *node = parse_expression(Prec0);
    elements.push_back(node);
    if (curr_token.type == Comma) {
      advance_token();
    } else if (curr_token.type == Rbracket) {
      break;
    } else {
      throw ParseError("expected , or ] in array expression");
    }
  }
  Node *node = new ArrayExpression(elements);
  return node;
}

Node *Parser::parse_member_expression() {
  std::vector<Node *> elements = {};
  Identifier *object = new Identifier(curr_token.literal);
  advance_token();
  advance_token();
  Node *property = parse_expression(Prec0);
  advance_token();
  // std::cout << "after parsing member = " << curr_token << "\n";
  Node *node = new MemberExpression(object, property);
  return node;
}
