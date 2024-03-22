#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  UNode &preproc(UNode &dest, Ast &ast, const UNode &src);
}
