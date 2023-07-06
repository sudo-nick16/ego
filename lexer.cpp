#include "lexer.h"

Lexer::Lexer(std::string source) : source(source) { read_char(); };

char Lexer::peek_char() { return source[read_pos]; }

void Lexer::read_char() {
  if (read_pos >= source.length()) {
    pos = read_pos;
  } else {
    pos = read_pos;
    read_pos++;
  }
}
std::vector<Token> Lexer::lex() {
  std::vector<Token> tokens;
  while (true) {
    while (isblank(source[pos])) {
      read_char();
    }
    switch (source[pos]) {
    case ';': {
      tokens.push_back(Token{Semicolon, ";"});
      break;
    }
    case ',': {
      tokens.push_back(Token{Comma, ","});
      break;
    }
    case '+': {
      tokens.push_back(Token{Plus, "+"});
      break;
    }
    case '-': {
      if (isdigit(peek_char())) {
        std::string number;
        int dot_count = 0;
        number += source[pos];
        read_char();
        while ((isdigit(source[pos]) || source[pos] == '.') && dot_count < 2) {
          number += source[pos];
          read_char();
        }
        tokens.push_back(Token{Number, number});
        break;
      }
      tokens.push_back(Token{Minus, "-"});
      break;
    }
    case '/': {
      tokens.push_back(Token{Div, "/"});
      break;
    }
    case '*': {
      tokens.push_back(Token{Mul, "*"});
      break;
    }
    case '^': {
      tokens.push_back(Token{Pow, "^"});
      break;
    }
    case '=': {
      if (peek_char() == '=') {
        tokens.push_back(Token{Equal, "=="});
        read_char();
        break;
      }
      tokens.push_back(Token{Assign, "="});
      break;
    }
    case '!': {
      if (peek_char() == '=') {
        tokens.push_back(Token{NotEqual, "!="});
        read_char();
        break;
      }
      tokens.push_back(Token{Bang, "!"});
      break;
    }
    case '<': {
      if (peek_char() == '=') {
        tokens.push_back(Token{Lte, "<="});
        read_char();
        break;
      }
      tokens.push_back(Token{Lt, "<"});
      break;
    }
    case '>': {
      if (peek_char() == '=') {
        tokens.push_back(Token{Gte, ">="});
        read_char();
        break;
      }
      tokens.push_back(Token{Gt, ">"});
      break;
    }
    case '(': {
      tokens.push_back(Token{Lparen, "("});
      break;
    }
    case ')': {
      tokens.push_back(Token{Rparen, ")"});
      break;
    }
    case '{': {
      tokens.push_back(Token{Lbrace, "{"});
      break;
    }
    case '}': {
      tokens.push_back(Token{Rbrace, "}"});
      break;
    }
    case '%': {
      tokens.push_back(Token{Mod, "%"});
      break;
    }
    case '"': {
      std::string str;
      read_char();
      while (source[pos] != '"') {
        str += source[pos];
        read_char();
      }
      tokens.push_back(Token{String, str});
      break;
    }
    case '\0': {
      tokens.push_back(Token{Eof, "Eof"});
      return tokens;
    }

    default: {
      if (isdigit(source[pos])) {
        std::string number;
        int dot_count = 0;
        while ((isdigit(source[pos]) || source[pos] == '.') && dot_count < 2) {
          number += source[pos];
          read_char();
        }
        tokens.push_back(Token{Number, number});
        continue;
      } else if (isalpha(source[pos])) {
        std::string identifier;
        while (isalpha(source[pos]) || source[pos] == '_') {
          identifier += source[pos];
          read_char();
        }
        if (Keywords.find(identifier) != Keywords.end()) {
          tokens.push_back(Token{
              Keywords.find(identifier)->second,
              identifier,
          });
          continue;
        }
        tokens.push_back(Token{
            Ident,
            identifier,
        });
        continue;
      }
    }
    }
    read_char();
  }
  return tokens;
};
