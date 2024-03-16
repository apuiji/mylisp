#pragma once

#include"ast.hh"

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

  bool isNumber(double &dest, std::string_view src);

  int parse(UNodes &dest, const char *start, const char *end);

  struct RawAtom: Node {
    std::string_view raw;
    RawAtom(const char *start, std::string_view raw) noexcept: Node(start), raw(raw) {}
  };

  struct NumberAtom final: RawAtom {
    double value;
    NumberAtom(const char *start, std::string_view raw, double value) noexcept: RawAtom(start, raw), value(value) {}
  };

  struct CharAtom final: Node {
    wchar_t value;
    CharAtom(const char *start, char value) noexcept: Node(start), value(value) {}
  };

  struct StringAtom final: Node {
    const std::string *value;
    StringAtom(const char *start, const std::string *value) noexcept: Node(start), value(value) {}
  };

  struct IDAtom final: RawAtom {
    using RawAtom::RawAtom
  };

  struct TokenAtom final: RawAtom {
    uint64_t token;
    TokenAtom(const Pos *pos, std::string_view raw, uint64_t token) noexcept: RawAtom(pos, raw), token(token) {}
  };

  struct List final: Node {
    UNode first;
    List(const Pos *pos, UNode &&first = {}) noexcept: Node(pos), first(std::move(first)) {}
  };
}
