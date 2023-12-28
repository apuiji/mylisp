#pragma once

#include"ast_include.hh"

namespace zlt::mylisp::ast {
  UNode &preproc(UNode &dest, const UNode &src);

  struct PreprocBad {
    const Pos *pos;
    std::string what;
    PreprocBad(const Pos *pos, std::string &&what) noexcept: pos(pos), what(std::move(what)) {}
  };
}
