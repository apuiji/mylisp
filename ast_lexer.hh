#pragma once

#include<string>
#include<utility>

namespace zlt::mylisp::ast {
  struct Lexer {
    double numval;
    int charval;
    std::string strval;
    std::wstring wstrval;
    std::wstring_view raw;
    std::tuple<uint64_t, const wchar_t *, const wchar_t *> operator ()(const wchar_t *it, const wchar_t *end);
  };

  struct LexerBad {
    const wchar_t *start;
    std::string what;
    LexerBad(const wchar_t *start, std::string &&what) noexcept: start(start), what(std::move(what)) {}
  };
}
