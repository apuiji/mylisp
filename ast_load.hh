#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  /// @param file canonicalized
  ItSource load(Ast &ast, const char *start, std::filesystem::path &&file);
}
