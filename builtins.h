#include "common.h"
#include "ast.h"

#ifndef builtin_functions_h
#define builtin_functions_h

extern const std::unordered_map<std::string, Color> GetRaylibColor;

extern const std::unordered_map<std::string, KeyboardKey> GetRaylibKey;

extern const std::unordered_map<std::string, TraceLogLevel> GetRaylibLogLevel;

extern const std::unordered_map<std::string,
                   std::function<Object *(Node *, Environment *env)>>
    BuiltinFunctions;

#endif // !builtin_functions_h
