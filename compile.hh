#pragma once

#include"ast_trans2.hh"

namespace zlt::mylisp {
  int compile(std::string &dest, const ast::UNode &src);
}
