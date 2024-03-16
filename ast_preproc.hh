#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  UNode &preproc(UNode &dest, Ast &ast, const UNode &src);

  struct PreprocBad {
    std::string what;
    std::vector<const Pos *> posk;
    PreprocBad(std::string &&what) noexcept: what(std::move(what)) {}
    PreprocBad(std::string &&what, std::vector<const Pos *> &&posk) noexcept: what(std::move(what)), posk(std::move(posk)) {}
    PreprocBad(std::string &&what, const Pos *pos): what(std::move(what)) {
      posk.push_back(pos);
    }
    PreprocBad(PreprocBad &&bad, const Pos *pos): PreprocBad(std::move(bad)) {
      posk.push_back(pos);
    }
  };
}
