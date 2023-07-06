#include "tokens.h"
#include <string>

Token::Token(TokenType t, std::string lit) : type(t), literal(lit) {}
std::string Token::to_string() {
  return "{\n\"type\": \"" + std::to_string(type) + "\",\n\"operator\": \"" +
         literal + "\"\n}";
}

std::ostream &operator<<(std::ostream &os, const Token &tok) {
  std::string str = "{" + std::to_string(tok.type) + ", " + tok.literal + "}";
  os << str;
  return os;
}

const std::unordered_map<std::string, TokenType> Keywords = {
    {"let", Let},
    {"if", If},
    {"else", Else},
    {"while", While},
    {"and", And},
    {"or", Or},
    {"return", Return},
    {"true", True},
    {"false", False},
    {"func", Function}
};
