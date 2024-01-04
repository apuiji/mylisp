#pragma once

#include<string>

namespace zlt::mylisp {
  int eval(const char *it, const char *end);

  struct EvalBad {
    std::string what;
    EvalBad(std::string &&what) noexcept: what(std::move(what)) {}
  };
}
