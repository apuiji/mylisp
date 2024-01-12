#pragma once

#include<string>

namespace zlt::mylisp {
  int eval(const char *it, const char *end);

  struct EvalBad {
    std::wstring what;
    EvalBad(std::wstring &&what) noexcept: what(std::move(what)) {}
  };
}
