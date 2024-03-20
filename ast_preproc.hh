#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  int preproc(UNodes &dest, Ast &ast, UNodes::const_iterator it, UNodes::const_iterator end);
}
