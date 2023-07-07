#include "ast.h"
#include "lexer.h"
#include "utils.h"

Environment::Environment() {}

Object *Environment::get_identifier(std::string name) {
  if (store.find(name) != store.end()) {
    return store[name];
  }
  return nullptr;
}

void Environment::set_identifier(std::string name, Object *obj) {
  store[name] = obj;
}

Literal::Literal(std::string value, DataType data_type)
    : value(value), data_type(data_type){};
std::string Literal::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"value\": \"" + value +
         "\",\n\"data_type\": \"" + std::to_string(data_type) + "\"\n}";
}
std::string Literal::statement_type() { return type; };

Identifier::Identifier(std::string name) : name(name){};
std::string Identifier::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"name\": \"" + name + "\"\n}";
}
std::string Identifier::statement_type() { return type; };

IntegerObject::IntegerObject(int value) : value(value){};
DataType IntegerObject::type() { return IntType; };
std::string IntegerObject::inspect() { return std::to_string(value); }
bool IntegerObject::is_truthy() { return value != 0; }

FloatObject::FloatObject(float value) : value(value) {}
DataType FloatObject::type() { return FloatType; }
std::string FloatObject::inspect() { return std::to_string(value); }
bool FloatObject::is_truthy() { return value != 0; }

StringObject::StringObject(std::string value) : value(value) {}
DataType StringObject::type() { return StringType; }
std::string StringObject::inspect() { return value; }
bool StringObject::is_truthy() { return value != ""; }

BoolObject::BoolObject(bool value) : value(value) {}
DataType BoolObject::type() { return BoolType; }
std::string BoolObject::inspect() { return std::to_string(value); }
bool BoolObject::is_truthy() { return value; }

FunctionObject::FunctionObject(std::vector<Node *> &body,
                               std::vector<std::string> &params)
    : body(body), params(params) {}

BinaryExpression::BinaryExpression(Node *left, Node *right, Token op)
    : left(left), right(right), op(op){};
std::string BinaryExpression::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"left\": " + left->to_string() +
         ",\n\"right\": " + right->to_string() +
         ",\n\"operator\": " + op.to_string() + "\n}";
}
std::string BinaryExpression::statement_type() { return type; };

LetStatement::LetStatement(Identifier ident, Node *value)
    : ident(ident), value(value) {}
std::string LetStatement::statement_type() { return type; };
std::string LetStatement::to_string() {
  return "{\n\"type\": \"" + type +
         "\",\n\"identifier\": " + ident.to_string() +
         ",\n\"value\": " + value->to_string() + "\n}";
}

IfStatement::IfStatement(Node *condition, std::vector<Node *> consequent,
                         std::vector<Node *> alternate)
    : condition(condition), consequent(consequent), alternate(alternate){};
std::string IfStatement::to_string() {
  return "{\n\"type\": \"" + type +
         "\",\n\"condition\": " + condition->to_string() +
         ",\n\"consequent\": " + nodes_to_str(consequent) +
         ",\n\"alternate\":" + nodes_to_str(alternate) + "\n}";
};
std::string IfStatement::statement_type() { return type; };

ReturnStatement::ReturnStatement(Node *value) : value(value){};
std::string ReturnStatement::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"value\": " + value->to_string() +
         "\n}";
};
std::string ReturnStatement::statement_type() { return type; };

FunctionStatement::FunctionStatement(Identifier ident,
                                     std::vector<Identifier *> params,
                                     std::vector<Node *> block)
    : params(params), ident(ident), block(block){};
std::string FunctionStatement::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"ident\": " + ident.to_string() +
         ",\n\"params\": " + nodes_to_str(params) +
         ",\n\"block\": " + nodes_to_str(block) + "\n}";
};
std::string FunctionStatement::statement_type() { return type; };

CallExpression::CallExpression(Identifier callee, std::vector<Node *> args)
    : callee(callee), args(args){};
std::string CallExpression::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"callee\": " + callee.to_string() +
         ",\n\"args\": " + nodes_to_str(args) + "\n}";
};
std::string CallExpression::statement_type() { return type; };

WhileStatement::WhileStatement(Node *condition, std::vector<Node *> block)
    : condition(condition), block(block){};
std::string WhileStatement::to_string() {
  return "{\n\"type\": \"" + type +
         "\",\n\"condition\": " + condition->to_string() +
         ",\n\"block\": " + nodes_to_str(block) + "\n}";
};
std::string WhileStatement::statement_type() { return type; };

AssignmentExpression::AssignmentExpression(Identifier ident, Node *value)
    : ident(ident), value(value) {}
std::string AssignmentExpression::statement_type() { return type; };
std::string AssignmentExpression::to_string() {
  return "{\n\"type\": \"" + type +
         "\",\n\"identifier\": " + ident.to_string() +
         ",\n\"value\": " + value->to_string() + "\n}";
}

ArrayExpression::ArrayExpression(std::vector<Node *> ele) : elements(ele) {}
std::string ArrayExpression::statement_type() { return type; };
std::string ArrayExpression::to_string() {
  return "{\n\"type\": \"" + type +
         "\",\n\"elements\": " + nodes_to_str(elements) + "\n}";
}

ArrayObject::ArrayObject(std::vector<Object *> ele) : elements(ele) {}
bool ArrayObject::is_truthy() { return elements.size() != 0; }
DataType ArrayObject::type() { return ArrayType; }
std::string ArrayObject::inspect() {
  std::string str = "[";
  for (int i = 0; i < elements.size(); i++) {
    str += elements[i]->inspect();
    if (i != elements.size() - 1) {
      str += ", ";
    }
  }
  str += "]";
  return str;
}

MemberExpression::MemberExpression(Node *obj, Node *prop)
    : object(obj), property(prop){};
std::string MemberExpression::statement_type() { return type; };
std::string MemberExpression::to_string() {
  return "{\n\"type\": \"" + type + "\",\n\"object\": " + object->to_string() +
         ",\n\"property\": " + property->to_string() + "\n}";
};
