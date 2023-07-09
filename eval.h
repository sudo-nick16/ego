#include "ast.h"
#include "common.h"
#include "tokens.h"

#ifndef eval_h
#define eval_h

class EvalError : public std::exception {
public:
  std::string error_msg;
  EvalError(std::string err);
  const char *what() const noexcept override;
};

template <typename VarType>
VarType evaluate_primary_op(VarType x, VarType y, TokenType op) {
  switch (op) {
  case Plus: {
    return x + y;
  }
  case Minus: {
    return x - y;
  }
  case Mul: {
    return x * y;
  }
  case Div: {
    return x / y;
  }
  case Mod: {
    return (int)x % (int)y;
  }
  case And: {
    return x && y;
  }
  case Or: {
    return x || y;
  }
  case Lt: {
    return x < y;
  }
  case Lte: {
    return x <= y;
  }
  case Gt: {
    return x > y;
  }
  case Gte: {
    return x >= y;
  }
  case Equal: {
    return x == y;
  }
  case NotEqual: {
    return x != y;
  }
  default: {
    throw EvalError("unknown operator");
  }
  }
  std::cout << x << " " << y << op << "\n";
  throw EvalError("unknown operator");
}

Object *evaluate_operator(Object *left, Object *right, Token op);

Object *evaluate(std::vector<Node *> program, Environment *env);

Object *evaluate_expression(Node *node, Environment *env);

#endif // !eval_h
