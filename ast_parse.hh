#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  int parse(UNode &dest, const char *it, const char *end);

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
    uint64_t token;
    TokenAtom(const char *start, std::string_view raw, uint64_t token) noexcept: RawAtom(start, raw), token(token) {}
  };

  struct List final: Node {
    UNode first;
    List(const char *start, UNode &&first = {}) noexcept: Node(start), first(std::move(first)) {}
  };

  int clone(UNode &dest, const UNode &src);
  UNode &clones(UNode &dest, const UNode &src);
}
