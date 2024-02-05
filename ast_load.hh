#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  /// @param file canonicalized
  Ast::ItLoaded load(Ast &ast, std::filesystem::path &&file);

  struct LoadBad {
    std::string what;
    LoadBad(std::string &&what) noexcept: what(std::move(what)) {}
  };
}
