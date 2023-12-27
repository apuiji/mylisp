#pragma once

#include"ast_include.hh"

namespace zlt::mylisp::ast {
  UNode &preproc(UNode &dest, const std::filesystem::path &file, const UNode &src);

  struct PreprocBad {
    const Pos &pos;
    std::string what;
  };
}
