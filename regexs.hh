#pragma once

#include<regex>
#include"object.hh"

namespace zlt::mylisp {
  struct RegexObj: Object {
    std::wregex<C> regex;
    RegexObj(std::wregex<C> &&regex) noexcept: regex(std::move(regex)) {}
  };

  Value natFnRegcomp(const Value *it, const Value *end);
  Value natFnRegexec(const Value *it, const Value *end);
}
