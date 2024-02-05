#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  int parse(UNode &dest, Ast &ast, const std::filesystem::path *file, const char *start, const char *end);

  struct ParseBad {
    Pos pos;
    std::string what;
    ParseBad(const Pos &pos, std::string &&what) noexcept: pos(pos), what(std::move(what)) {}
  };

  struct RawAtom: Node {
    std::string_view raw;
    RawAtom(const Pos *pos, std::string_view raw) noexcept: Node(pos), raw(raw) {}
  };

  struct NumberAtom final: RawAtom {
    double value;
    NumberAtom(const Pos *pos, std::string_view raw, double value) noexcept: RawAtom(pos, raw), value(value) {}
  };

  struct CharAtom final: Node {
    wchar_t value;
    CharAtom(const Pos *pos, char value) noexcept: Node(pos), value(value) {}
  };

  struct StringAtom final: Node {
    const std::string *value;
    StringAtom(const Pos *pos, const std::string *value) noexcept: Node(pos), value(value) {}
  };

  struct IDAtom final: Node {
    const std::string *name;
    IDAtom(const Pos *pos, const std::string *name) noexcept: Node(pos), name(name) {}
  };

  struct TokenAtom final: RawAtom {
    uint64_t token;
    TokenAtom(const Pos *pos, std::string_view raw, uint64_t token) noexcept: RawAtom(pos, raw), token(token) {}
  };

  struct List final: Node {
    UNode first;
    List(const Pos *pos, UNode &&first = {}) noexcept: Node(pos), first(std::move(first)) {}
  };

  int clone(UNode &dest, const UNode &src);
  UNode &clones(UNode &dest, const UNode &src);
}
