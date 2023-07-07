#include "builtins.h"
#include "common.h"
#include "eval.h"
#include "utils.h"

const std::unordered_map<std::string, Color> GetRaylibColor = {
    {"white", WHITE}, {"black", BLACK}, {"pink", PINK},
    {"blue", BLUE},   {"green", GREEN},
};

const std::unordered_map<std::string, KeyboardKey> GetRaylibKey = {
    {"down", KEY_DOWN}, {"up", KEY_UP},       {"right", KEY_RIGHT},
    {"left", KEY_LEFT}, {"space", KEY_SPACE},
};

const std::unordered_map<std::string, TraceLogLevel> GetRaylibLogLevel = {
    {"log_all", LOG_ALL},         {"log_trace", LOG_TRACE},
    {"log_debug", LOG_DEBUG},     {"log_info", LOG_INFO},
    {"log_warning", LOG_WARNING}, {"log_error", LOG_ERROR},
    {"log_fatal", LOG_FATAL},     {"log_none", LOG_NONE}};

const std::unordered_map<std::string,
                         std::function<Object *(Node *, Environment *env)>>
    BuiltinFunctions = {
        {"print",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           for (int i = 0; i < callNode->args.size(); ++i) {
             if (callNode->args[i]->statement_type() == "ArrayExpression") {
               std::cout << nodes_to_str(
                   ((ArrayExpression *)callNode->args[i])->elements);
               continue;
             }
             Object *obj = evaluate_expression(callNode->args[i], global_env);
             std::cout << obj->inspect()
                       << ((i == callNode->args.size() - 1) ? "" : " ");
           }
           return nullptr;
         }},
        {"rand_int",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           for (auto arg : callNode->args) {
             Object *obj = evaluate_expression(arg, global_env);
             std::cout << obj->inspect() << " ";
           }
           IntegerObject *obj = new IntegerObject(rand());
           return obj;
         }},
        {"println",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           for (int i = 0; i < callNode->args.size(); ++i) {
             if (callNode->args[i]->statement_type() == "ArrayExpression") {
               std::cout << nodes_to_str(
                   ((ArrayExpression *)callNode->args[i])->elements);
               continue;
             }
             Object *obj = evaluate_expression(callNode->args[i], global_env);
             std::cout << obj->inspect()
                       << ((i == callNode->args.size() - 1) ? "" : " ");
           }
           std::cout << "\n";
           return nullptr;
         }},
        {"len",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           Object *obj = evaluate_expression(callNode->args[0], global_env);
           if (obj->type() != StringType) {
             throw EvalError("invalid argument type, expected string");
           }
           Object *ret = new IntegerObject(((StringObject *)obj)->value.size());
           return ret;
         }},
        {"ceil",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           Object *obj = evaluate_expression(callNode->args[0], global_env);
           Object *ret;
           if (obj->type() == FloatType) {
             ret = new IntegerObject((int)ceil(((FloatObject *)obj)->value));
           } else if (obj->type() == IntType) {
             ret = new IntegerObject(((IntegerObject *)obj)->value);
           } else {
             throw EvalError("invalid argument type, expected float or int");
           }
           return ret;
         }},
        {"floor",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           Object *obj = evaluate_expression(callNode->args[0], global_env);
           Object *ret;
           if (obj->type() == FloatType) {
             ret = new IntegerObject((int)floor(((FloatObject *)obj)->value));
           } else if (obj->type() == IntType) {
             ret = new IntegerObject(((IntegerObject *)obj)->value);
           } else {
             throw EvalError("invalid argument type, expected float or int");
           }
           return ret;
         }},
        {"make_window",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 3) {
             throw EvalError("invalid number of arguments");
           }
           int width = ((IntegerObject *)evaluate_expression(callNode->args[0],
                                                             global_env))
                           ->value;
           int height = ((IntegerObject *)evaluate_expression(callNode->args[1],
                                                              global_env))
                            ->value;

           std::string title = ((StringObject *)evaluate_expression(
                                    callNode->args[2], global_env))
                                   ->value;
           InitWindow(width, height, title.c_str());
           return nullptr;
         }},
        {"begin_drawing",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 0) {
             throw EvalError("invalid number of arguments");
           }
           BeginDrawing();
           return nullptr;
         }},
        {"end_drawing",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 0) {
             throw EvalError("invalid number of arguments");
           }
           EndDrawing();
           return nullptr;
         }},
        {"windows_should_close",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 0) {
             throw EvalError("invalid number of arguments");
           }
           BoolObject *obj = new BoolObject(WindowShouldClose());
           return obj;
         }},
        {"close_window",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 0) {
             throw EvalError("invalid number of arguments");
           }
           CloseWindow();
           return nullptr;
         }},
        {"to_int",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           float val = ((FloatObject *)evaluate_expression(callNode->args[0],
                                                           global_env))
                           ->value;
           IntegerObject *obj = new IntegerObject(floor(val));
           return obj;
         }},
        {"to_str",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           std::string s =
               evaluate_expression(callNode->args[0], global_env)->inspect();

           StringObject *obj = new StringObject(s);
           return obj;
         }},
        {"wait_time",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           float time = ((FloatObject *)evaluate_expression(callNode->args[0],
                                                            global_env))
                            ->value;
           WaitTime(time / 1000.0);
           return nullptr;
         }},
        {"clr_bg",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           std::string color = ((StringObject *)evaluate_expression(
                                    callNode->args[0], global_env))
                                   ->value;
           if (GetRaylibColor.find(color) == GetRaylibColor.end()) {
             throw EvalError("invalid color");
           }
           ClearBackground(GetRaylibColor.find(color)->second);
           return nullptr;
         }},
        {"draw_rec",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 5) {
             throw EvalError("invalid number of arguments");
           }
           int posx = (int)((IntegerObject *)evaluate_expression(
                                callNode->args[0], global_env))
                          ->value;

           int posy = (int)((IntegerObject *)evaluate_expression(
                                callNode->args[1], global_env))
                          ->value;
           int width = (int)((IntegerObject *)evaluate_expression(
                                 callNode->args[2], global_env))
                           ->value;
           int height = (int)((IntegerObject *)evaluate_expression(
                                  callNode->args[3], global_env))
                            ->value;
           std::string color = ((StringObject *)evaluate_expression(
                                    callNode->args[4], global_env))
                                   ->value;
           if (GetRaylibColor.find(color) == GetRaylibColor.end()) {
             throw EvalError("invalid color");
           }
           DrawRectangle(posx, posy, width, height,
                         GetRaylibColor.find(color)->second);
           return nullptr;
         }},
        {"draw_text",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 5) {
             throw EvalError("invalid number of arguments");
           }
           std::string text = ((StringObject *)evaluate_expression(
                                   callNode->args[0], global_env))
                                  ->value;
           int posx = (int)((IntegerObject *)evaluate_expression(
                                callNode->args[1], global_env))
                          ->value;

           int posy = (int)((IntegerObject *)evaluate_expression(
                                callNode->args[2], global_env))
                          ->value;
           int font_size = (int)((IntegerObject *)evaluate_expression(
                                     callNode->args[3], global_env))
                               ->value;
           std::string color = ((StringObject *)evaluate_expression(
                                    callNode->args[4], global_env))
                                   ->value;
           if (GetRaylibColor.find(color) == GetRaylibColor.end()) {
             throw EvalError("invalid color");
           }
           DrawText(text.c_str(), posx, posy, font_size,
                    GetRaylibColor.find(color)->second);
           return nullptr;
         }},
        {"draw_circle",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 4) {
             throw EvalError("invalid number of arguments");
           }
           int centerX = ((IntegerObject *)evaluate_expression(
                              callNode->args[0], global_env))
                             ->value;

           int centerY = ((IntegerObject *)evaluate_expression(
                              callNode->args[1], global_env))
                             ->value;
           float radius = (float)((FloatObject *)evaluate_expression(
                                      callNode->args[2], global_env))
                              ->value;
           std::string color = ((StringObject *)evaluate_expression(
                                    callNode->args[3], global_env))
                                   ->value;
           if (GetRaylibColor.find(color) == GetRaylibColor.end()) {
             throw EvalError("invalid color");
           }
           DrawCircle(centerX, centerY, radius,
                      GetRaylibColor.find(color)->second);
           return nullptr;
         }},
        {"is_key_down",
         [](Node *node, Environment *global_env) -> Object * {
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           std::string key = ((StringObject *)evaluate_expression(
                                  callNode->args[0], global_env))
                                 ->value;
           if (GetRaylibKey.find(key) == GetRaylibKey.end()) {
             throw EvalError("invalid key");
           }
           BoolObject *obj =
               new BoolObject(IsKeyDown(GetRaylibKey.find(key)->second));
           return obj;
         }},
        {"set_log_level",
         [](Node *node, Environment *global_env) -> Object * {
           SetTraceLogLevel(LOG_NONE);
           CallExpression *callNode = (CallExpression *)node;
           if (callNode->args.size() != 1) {
             throw EvalError("invalid number of arguments");
           }
           std::string key = ((StringObject *)evaluate_expression(
                                  callNode->args[0], global_env))
                                 ->value;
           if (GetRaylibLogLevel.find(key) == GetRaylibLogLevel.end()) {
             throw EvalError("invalid key");
           }
           SetTraceLogLevel(GetRaylibLogLevel.find(key)->second);
           return nullptr;
         }},

};
