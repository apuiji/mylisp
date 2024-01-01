#pragma once

#include"ast_trans2.hh"

namespace zlt::mylisp::ast {
  int compile(std::string &dest, const UNode &src);
}
