#pragma once

#include<deque>
#include<map>
#include"value.hh"

namespace zlt::mylisp {
  struct Object {
    enum {
      WHITE_COLOR,
      GRAY_COLOR,
      BLACK_COLOR
    };
    Object *prev;
    Object *next;
    int color;
    virtual ~Object() = default;
  };

  template<class C>
  struct BasicStringObj final: Object {
    std::basic_string<C> value;
    BasicStringObj(const std::basic_string<C> &value) noexcept: value(value) {}
    BasicStringObj(std::basic_string<C> &&value) noexcept: value(std::move(value)) {}
  };

  using StringObj = BasicStringObj<wchar_t>;
  using Latin1Obj = BasicStringObj<char>;

  struct FunctionObj final: Object {
    std::map<const std::wstring *, Value> closures;
    const std::string &body;
    FunctionObj(const std::string &body) noexcept: body(body) {}
  };

  struct ListObj final: Object {
    std::deque<Value> list;
  };

  struct SetObj final: Object {
    struct Comparator {
      bool operator ()(const Value &a, const Value &b) noexcept {
        int diff;
        return !compare(diff, a, b) || diff < 0;
      }
    };
    std::set<Value, Comparator>;
  };

  struct MapObj final: Object {
    std::map<Value, Value, SetObj::Comparator> map;
  };
}
