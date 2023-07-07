#include "eval.h"
#include "builtins.h"
#include "utils.h"

EvalError::EvalError(std::string err)
    : error_msg("error while evaluating: " + err) {}
const char *EvalError::what() const noexcept { return error_msg.c_str(); }

Object *evaluate_operator(Object *left, Object *right, Token op) {
  // if (left->type() != right->type()) {
  // std::cout << "WARNING: type mismatch while operating\n";
  // throw EvalError("type mismatch while operating");
  // }
  if ((left->type() == StringType || right->type() == StringType) &&
      op.type == Plus) {
    return new StringObject(left->inspect() + right->inspect());
  } else if (left->type() == FloatType || right->type() == FloatType) {
    FloatObject *left_float;
    if (op.type == Mod) {
      throw EvalError("cannot use % on floats");
    }
    return new FloatObject(evaluate_primary_op(
        std::stof(left->inspect()), std::stof(right->inspect()), op.type));
  } else if (left->type() == IntType || right->type() == IntType) {
    return new IntegerObject(evaluate_primary_op(
        std::stoi(left->inspect()), std::stoi(right->inspect()), op.type));
  } else if (left->type() == BoolType || right->type() == BoolType) {
    return new IntegerObject(evaluate_primary_op(
        (int)left->is_truthy(), (int)right->is_truthy(), op.type));
  }
  throw EvalError("unknown operator");
  return nullptr;
}

Object *evaluate_expression(Node *node, Environment *env) {
  if (node->statement_type() == "BinaryExpression") {
    BinaryExpression *bNode = (BinaryExpression *)node;
    Object *left = evaluate_expression(bNode->left, env);
    Object *right = evaluate_expression(bNode->right, env);
    return evaluate_operator(left, right, bNode->op);
  } else if (node->statement_type() == "Literal") {
    Object *obj = get_obj_from_literal((Literal *)node);
    if (obj == nullptr) {
      throw EvalError("invalid literal type " + ((Literal *)node)->type);
    }
    return obj;
  } else if (node->statement_type() == "Identifier") {
    Object *obj = env->get_identifier(((Identifier *)node)->name);
    if (obj == nullptr) {
      throw EvalError("undefined identifier: " + ((Identifier *)node)->name);
    }
    return obj;
  } else if (node->statement_type() == "CallExpression") {
    CallExpression *callNode = (CallExpression *)node;
    if (BuiltinFunctions.find(callNode->callee.name) !=
        BuiltinFunctions.end()) {
      return BuiltinFunctions.find(callNode->callee.name)->second(node, env);
    }
    if (env->functions.find(callNode->callee.name) == env->functions.end()) {
      throw EvalError("function " + callNode->callee.name + " not defined");
    }
    FunctionObject *funcObj =
        (FunctionObject *)env->functions[callNode->callee.name];
    if (callNode->args.size() != funcObj->params.size()) {
      throw EvalError("invalid number of arguments");
    }
    int i = 0;
    Environment *func_env = new Environment();
    for (auto param : funcObj->params) {
      func_env->store[param] = evaluate_expression(callNode->args[i], env);
    }
    return evaluate(funcObj->body, func_env);
  } else if (node->statement_type() == "MemberExpression") {
    MemberExpression *memNode = (MemberExpression *)node;
    std::string object = ((Identifier *)memNode->object)->name;
    if (env->store.find(object) == env->store.end()) {
      throw EvalError("object not defined");
    }
    ArrayObject *value = (ArrayObject *)env->store[object];
    Object *prop = evaluate_expression(memNode->property, env);
    if (prop->type() != IntType) {
      throw EvalError("invalid property type");
    }
    int index = ((IntegerObject *)prop)->value;
    if (index < 0 || index >= value->elements.size()) {
      throw EvalError("index out of bounds");
    }
    return value->elements[index];
  }
  throw EvalError("invalid initialization value " + node->statement_type());
  return nullptr;
}

Object *evaluate(std::vector<Node *> program, Environment *env) {
  for (auto node : program) {
    std::string type = node->statement_type();
    if (type == "LetStatement") {
      LetStatement *letNode = (LetStatement *)node;
      std::string name = letNode->ident.name;
      if (env->store.find(name) != env->store.end()) {
        throw EvalError("variable already defined: " + name);
      }
      if (letNode->value->statement_type() == "ArrayExpression") {
        ArrayExpression *arrNode = (ArrayExpression *)letNode->value;
        std::vector<Object *> arr;
        for (auto elem : arrNode->elements) {
          arr.push_back(evaluate_expression(elem, env));
        }
        env->store[name] = new ArrayObject(arr);
        continue;
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
      IfStatement *ifNode = (IfStatement *)node;
      // std::cout << "if statement\n";
      if (evaluate_expression(ifNode->condition, env)->is_truthy()) {
        evaluate(ifNode->consequent, env);
      } else if (ifNode->alternate.size() > 0) {
        evaluate(ifNode->alternate, env);
      }
    } else if (type == "FunctionStatement") {
      FunctionStatement *funcNode = (FunctionStatement *)node;
      std::string name = funcNode->ident.name;
      if (env->functions.find(name) != env->functions.end()) {
        throw EvalError("function already defined");
      }
      std::unordered_map<std::string, Object *> params;
      std::vector<std::string> params_vec;
      for (auto param : funcNode->params) {
        params_vec.push_back(param->name);
      }

      FunctionObject *funcObj = new FunctionObject(funcNode->block, params_vec);
      env->functions[name] = funcObj;
    } else if (type == "CallExpression") {
      evaluate_expression(node, env);
    } else if (type == "WhileStatement") {
      WhileStatement *whileNode = (WhileStatement *)node;
      while (evaluate_expression(whileNode->condition, env)->is_truthy()) {
        evaluate(whileNode->block, env);
      }
    } else if (type == "ReturnStatement") {
      ReturnStatement *retNode = (ReturnStatement *)node;
      return evaluate_expression(retNode->value, env);
    } else if (type == "MemberExpression") {
      return evaluate_expression(node, env);
    }
  }
  return nullptr;
  // std::cout << "-----\n";
  // for (auto ele : env->store) {
  //   std::cout << ele.first << " -> " << ele.second->inspect() << "\n";
  // }
}
