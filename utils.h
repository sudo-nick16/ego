#include "ast.h"
#include "common.h"
#include "tokens.h"

#ifndef utils_h
#define utils_h

bool is_binary_op(TokenType t);

bool is_token_type_bool(TokenType t);

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

Object *get_obj_from_literal(Literal *l);

#endif // !utils_h
