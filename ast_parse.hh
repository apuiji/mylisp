#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  struct Lexer {
    double numval;
    char charval;
    std::string strval;
    std::string_view raw;
    std::tuple<int, const char *, const char *> operator ()(const char *it, const char *end);
  };

  int parse(UNodes &dest, const char *it, const char *end);

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

  struct IDAtom final: Node {
    const std::string *name;
    IDAtom(const char *start, const std::string *name) noexcept: Node(start), name(name) {}
  };

  struct TokenAtom final: RawAtom {
    int token;
    TokenAtom(const char *start, std::string_view raw, int token) noexcept: RawAtom(start, raw), token(token) {}
  };

  struct List final: Node {
    UNodes items;
    List(const char *start, UNode &&items = {}) noexcept: Node(start), items(std::move(items)) {}
  };

  int clone(UNode &dest, const UNode &src);
  int clone(UNodes &dest, const UNode &src);
  int clones(UNodes &dest, UNodes::const_iterator it, UNodes::const_iterator end);
}
