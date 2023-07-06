#include "utils.h"

bool is_binary_op(TokenType t) {
  if (t == Plus || t == Minus || t == Div || t == Mul || t == Pow || t == Mod ||
      t == Lt || t == Gt || t == Lte || t == Gte || t == Equal ||
      t == NotEqual || t == PlusEq || t == MinusEq || t == DivEq ||
      t == MulEq || t == PowEq || t == And || t == Or) {
    return true;
  }
  return false;
}

bool is_token_type_bool(TokenType t) {
  if (t == True || t == False) {
    return true;
  }
  return false;
}


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
    throw nullptr;
  }
  }
  return new StringObject(l->value);
}
