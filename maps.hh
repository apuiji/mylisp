#pragma once

#include"myccutils/mymap.hh"
#include"object.hh"

namespace zlt::mylisp {
  struct MapObj final: Object {
    struct StrPoolComp {
      int operator ()(std::string_view x, const Value &b) const noexcept {
        std::string_view y;
        dynamicast(y, b);
        return x.compare(y);
      }
      int operator ()(const std::string *x, const Value &b) const noexcept {
        return operator ()((std::string_view) *x, b);
      }
    };
    using StrPool = mymap::Map<Value, Value, StrPoolComp>;
    std::pair<bool, Value> nullPool;
    std::pair<bool, Value> nanPool;
    std::map<double, Value> numPool;
    StrPool strPool;
    std::map<Object *, Value> objPool;
    std::map<void *, Value> ptrPool;
    // member operations begin
    Value objGetMemb(const Value &memb) const noexcept override;
    int objSetMemb(const Value &memb, const Value &value) override;
    // member operations end
    int graySubjs() noexcept override;
  };
}
