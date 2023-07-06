#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "eval.h"
#include "common.h"

int main(int argc, char **argv) {
  srand(time(0));
  if (argc == 1) {
    std::cout << "Usage: ego <filename>" << std::endl;
    return 0;
  }
  std::string filepath = argv[1];
  std::ifstream file(filepath);
  if (file.is_open()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.lex();
    Parser *parser = new Parser(tokens);
    std::vector<Node *> program = parser->parse(Eof);
    Environment *global_env = new Environment();
    evaluate(program, global_env);
  } else {
    std::cout << "Unable to open file" << std::endl;
  }
  return 0;
}
