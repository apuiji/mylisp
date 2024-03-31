#pragma once

#include"ast.hh"

namespace zlt::mylisp {
  int compile(std::string &dest, ast::UNodes::const_iterator it, ast::UNodes::const_iterator end);
}
