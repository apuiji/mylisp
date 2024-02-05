#pragma once

#include<string>
#include<utility>

namespace zlt::mylisp::ast {
  struct Lexer {
    double numval;
    char charval;
    std::string strval;
    std::string_view raw;
    std::tuple<uint64_t, const char *, const char *> operator ()(const char *it, const char *end);
  };

  struct LexerBad {
    const char *start;
    std::string what;
    LexerBad(const char *start, std::string &&what) noexcept: start(start), what(std::move(what)) {}
  };
}
