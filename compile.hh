#pragma once

#include"ast.hh"

namespace zlt::mylisp {
  int compile(std::string &dest, const ast::UNode &src);
}
