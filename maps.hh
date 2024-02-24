#pragma once

#include"myccutils/mymap.hh"
#include"object.hh"

namespace zlt::mylisp {
  struct MapObj final: Object {
    using StrPool = mymap::Node<Value, Value>;
    std::pair<bool, Value> nullPool;
    std::pair<bool, Value> nanPool;
    std::map<double, Value> numPool;
    StrPool *strPool;
    std::map<Object *, Value> objPool;
    std::map<void *, Value> ptrPool;
    // member operations begin
    Value objGetMemb(const Value &memb) const noexcept override;
    int objSetMemb(const Value &memb, const Value &value) override;
    // member operations end
    int graySubjs() noexcept override;
  };
}
