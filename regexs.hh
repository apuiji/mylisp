#pragma once

#include<regex>
#include"object.hh"

namespace zlt::mylisp {
  struct RegexObj: Object {
    std::wregex regex;
    RegexObj(std::wregex &&regex) noexcept: regex(std::move(regex)) {}
  };

  Value natFnRegcomp(const Value *it, const Value *end);
  Value natFnRegexec(const Value *it, const Value *end);
}
