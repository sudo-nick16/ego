#include "lexer.h"
#include "tokens.h"

#ifndef ast_h
#define ast_h

class Node {
public:
  virtual std::string statement_type() = 0;
  virtual std::string to_string() = 0;
};

class Literal : public Node {
public:
  Literal(std::string value, DataType data_type);
  std::string to_string();
  std::string statement_type();
  std::string type = "Literal";
  DataType data_type;
  std::string value;
};

class Identifier : public Node {
public:
  Identifier(std::string n);
  std::string to_string();
  std::string statement_type();
  std::string type = "Identifier";
  std::string name;
};

class Object {
public:
  virtual DataType type() = 0;
  virtual std::string inspect() = 0;
  virtual bool is_truthy() = 0;
};

class IntegerObject : public Object {
public:
  IntegerObject();
  IntegerObject(int val);
  DataType type();
  std::string inspect();
  bool is_truthy();
  int value;
};

class FloatObject : public Object {
public:
  FloatObject(float value);
  DataType type();
  std::string inspect();
  bool is_truthy();
  float value;
};

class StringObject : public Object {
public:
  StringObject();
  StringObject(std::string val);
  DataType type();
  std::string inspect();
  bool is_truthy();
  std::string value;
};

class BoolObject : public Object {
public:
  BoolObject();
  BoolObject(bool value);
  DataType type();
  std::string inspect();
  bool is_truthy();
  bool value;
};

class ArrayObject : public Object {
public:
  ArrayObject();
  ArrayObject(std::vector<Object *> ele);
  DataType type();
  std::string inspect();
  bool is_truthy();
  std::vector<Object *> elements;
};

class FunctionObject {
public:
  FunctionObject();
  FunctionObject(std::vector<Node *> &b, std::vector<std::string> &p);
  std::vector<std::string> params;
  std::vector<Node *> body;
};

class Environment {
public:
  Environment();
  std::unordered_map<std::string, Object *> store;
  std::unordered_map<std::string, FunctionObject *> functions;

  Object *get_identifier(std::string name);
  void set_identifier(std::string name, Object *obj);
};

class BinaryExpression : public Node {
public:
  BinaryExpression(Node *left, Node *right, Token op);
  std::string to_string();
  std::string statement_type();
  std::string type = "BinaryExpression";
  Node *left;
  Token op;
  Node *right;
};

class LetStatement : public Node {
public:
  LetStatement(Identifier ident, Node *value);
  std::string statement_type();
  std::string to_string();
  std::string type = "LetStatement";
  Identifier ident;
  Node *value;
  Environment *env;
};

class IfStatement : public Node {
public:
  IfStatement(Node *condition, std::vector<Node *> consequent,
              std::vector<Node *> alternate);
  std::string to_string();
  std::string statement_type();
  std::string type = "IfStatement";
  Node *condition;
  std::vector<Node *> consequent;
  std::vector<Node *> alternate;
};

class ReturnStatement : public Node {
public:
  ReturnStatement(Node *value);
  std::string to_string();
  std::string statement_type();
  std::string type = "ReturnStatement";
  Node *value;
};

class FunctionStatement : public Node {
public:
  FunctionStatement(Identifier ident, std::vector<Identifier *> params,
                    std::vector<Node *> block);
  std::string to_string();
  std::string statement_type();
  std::string type = "FunctionStatement";
  std::vector<Identifier *> params;
  Identifier ident;
  std::vector<Node *> block;
};

class CallExpression : public Node {
public:
  CallExpression(Identifier callee, std::vector<Node *> args);
  std::string to_string();
  std::string statement_type();
  std::string type = "CallExpression";
  Identifier callee;
  std::vector<Node *> args;
};

class WhileStatement : public Node {
public:
  WhileStatement(Node *condition, std::vector<Node *> block);
  std::string to_string();
  std::string statement_type();
  std::string type = "WhileStatement";
  Node *condition;
  std::vector<Node *> block;
};

class AssignmentExpression : public Node {
public:
  AssignmentExpression(Identifier ident, Node *value);
  std::string statement_type();
  std::string to_string();
  std::string type = "AssignmentExpression";
  Identifier ident;
  Node *value;
};

class ArrayExpression : public Node {
public:
  ArrayExpression(std::vector<Node *> elements);
  std::string statement_type();
  std::string to_string();
  std::string type = "ArrayExpression";
  std::vector<Node *> elements;
};

class MemberExpression : public Node {
public:
  MemberExpression(Node *object, Node *property);
  std::string statement_type();
  std::string to_string();
  std::string type = "MemberExpression";
  Node *object;
  Node *property;
};

#endif // !ast_h
