#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  UNode &parse(UNode &dest, const std::filesystem::path *file, const wchar_t *begin, const wchar_t *end);

  struct ParseBad {
    Pos pos;
    std::string what;
    ParseBad(const Pos &pos, std::string &&what) noexcept: pos(pos), what(std::move(what)) {}
  };

  template<class T>
  struct LiteralAtom final: Node {
    T value;
    LiteralAtom(const Pos *pos, T value) noexcept: Node(pos), value(value) {}
  };

  using NumberAtom = LiteralAtom<double>;
  using CharAtom = LiteralAtom<wchar_t>;
  using StringAtom = LiteralAtom<const std::wstring *>;
  using Latin1Atom = LiteralAtom<const std::string *>;

  struct IDAtom final: Node {
    const std::wstring *name;
    IDAtom(const Pos *pos, const std::wstring *name) noexcept: Node(pos), name(name) {}
  };

  struct TokenAtom final: Node {
    uint64_t token;
    TokenAtom(const Pos *pos, uint64_t token) noexcept: Node(pos), token(token) {}
  };

  struct List final: Node {
    UNode first;
    List(const Pos *pos, UNode &&first = {}) noexcept: Node(pos), first(std::move(first)) {}
  };
}
