#pragma once

#include<deque>
#include"object.hh"

namespace zlt::mylisp {
  struct ListObj final: Object {
    using List = std::deque<Value>;
    using Iterator = List::iterator;
    using ConstIterator = List::const_iterator;
    List list;
    ListObj(List &&list = {}) noexcept: list(std::move(list)) {}
    // member operations begin
    Value objGetMemb(const Value &memb) const noexcept override;
    int objSetMemb(const Value &memb, const Value &value) override;
    // member operations end
    int graySubjs() noexcept override;
  };
}
