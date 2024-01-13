#pragma once

#include"ast.hh"

namespace zlt::mylisp::ast {
  /// @param file canonicalized
  Ast::ItLoaded load(Ast &ast, std::filesystem::path &&file);

  struct LoadBad {
    std::wstring what;
    LoadBad(std::wstring &&what) noexcept: what(std::move(what)) {}
  };
}
