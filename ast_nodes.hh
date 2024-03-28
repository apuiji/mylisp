#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {

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
    const std::string *name;
    IDAtom(const char *start, std::string_view raw, const std::string *name) noexcept: RawAtom(start, raw), name(name) {}
  };

  struct TokenAtom final: RawAtom {
    int token;
    TokenAtom(const char *start, std::string_view raw, int token) noexcept: RawAtom(start, raw), token(token) {}
  };

  struct List final: Node {
    UNodes items;
    List(const char *start, UNodes &&items = {}) noexcept: Node(start), items(std::move(items)) {}
  };
}
