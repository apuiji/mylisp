#pragma once

#include<vector>
#include"ast.hh"

namespace zlt::mylisp {
  struct Macro {
    using Params = std::vector<const std::wstring *>;
    using ItParam = Params::const_iterator;
    Params params;
    ast::UNode body;
    Macro() = default;
    Macro(Params &&params, ast::UNode &&body) noexcept: params(std::move(params)), body(std::move(body)) {}
  };
}
