#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  UNode &preproc(UNode &dest, const UNode &src);

  struct PreprocBad {
    const Pos &pos;
    std::string what;
  };
}
