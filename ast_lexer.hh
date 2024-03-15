#pragma once

#include<string>
#include<utility>

namespace zlt::mylisp::ast {
  const char *hit(const char *it, const char *end) noexcept;

  struct Lexer {
    double numval;
    char charval;
    std::string strval;
    std::string_view raw;
    /// @return [token, end]
    std::tuple<uint64_t, const char *> operator ()(const char *it, const char *end);
  };

  struct LexerBad {
    int code;
    const char *start;
    LexerBad(int code, const char *start) noexcept: code(code), start(start) {}
  };
}
