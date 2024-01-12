#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  int parse(UNode &dest, const std::filesystem::path *file, const wchar_t *begin, const wchar_t *end);

  struct ParseBad {
    Pos pos;
    std::wstring what;
    ParseBad(const Pos &pos, std::wstring &&what) noexcept: pos(pos), what(std::move(what)) {}
  };

  struct RawAtom: Node {
    std::wstring_view raw;
    RawAtom(const Pos *pos, std::wstring_view raw) noexcept: Node(pos), raw(raw) {}
  };

  struct NumberAtom final: RawAtom {
    double value;
    NumberAtom(const Pos *pos, std::wstring_view raw, double value) noexcept: RawAtom(pos, raw), value(value) {}
  };

  struct CharAtom final: Node {
    wchar_t value;
    CharAtom(const Pos *pos, wchar_t value) noexcept: Node(pos), value(value) {}
  };

  struct StringAtom final: Node {
    const std::wstring *value;
    StringAtom(const Pos *pos, const std::wstring *value) noexcept: Node(pos), value(value) {}
  };

  struct IDAtom final: Node {
    const std::wstring *name;
    IDAtom(const Pos *pos, const std::wstring *name) noexcept: Node(pos), name(name) {}
  };

  struct TokenAtom final: RawAtom {
    uint64_t token;
    TokenAtom(const Pos *pos, std::wstring_view raw, uint64_t token) noexcept: RawAtom(pos, raw), token(token) {}
  };

  struct List final: Node {
    UNode first;
    List(const Pos *pos, UNode &&first = {}) noexcept: Node(pos), first(std::move(first)) {}
  };
}
