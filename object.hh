#pragma once

#include<deque>
#include<map>
#include<set>
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
    virtual int graySubjs() noexcept {
      return 0;
    }
  };

  template<class C>
  struct BasicStringObj final: Object {
    std::basic_string<C> value;
    BasicStringObj(const std::basic_string<C> &value) noexcept: value(value) {}
    BasicStringObj(std::basic_string<C> &&value) noexcept: value(std::move(value)) {}
  };

  using StringObj = BasicStringObj<wchar_t>;
  using Latin1Obj = BasicStringObj<char>;

  template<class C>
  struct BasicStringViewObj final: Object {
    std::basic_string_view<C> value;
    BasicStringViewObj(std::basic_string_view<C> value) noexcept: value(value) {}
  };

  using StringViewObj = BasicStringViewObj<wchar_t>;
  using Latin1ViewObj = BasicStringViewObj<char>;

  struct FunctionObj final: Object {
    std::map<const std::wstring *, Value> closures;
    const std::string &body;
    FunctionObj(const std::string &body) noexcept: body(body) {}
    int graySubjs() noexcept override;
  };

  struct ListObj final: Object {
    std::deque<Value> list;
    int graySubjs() noexcept override;
  };

  struct SetObj final: Object {
    struct Comparator {
      bool operator ()(const Value &a, const Value &b) const noexcept;
    };
    std::set<Value, Comparator> set;
    int graySubjs() noexcept override;
  };

  struct MapObj final: Object {
    std::map<Value, Value, SetObj::Comparator> map;
    int graySubjs() noexcept override;
  };

  struct PointerObj final: Object {
    Value *value;
    Value &operator *() noexcept {
      return *value;
    }
    const Value &operator *() const noexcept {
      return *value;
    }
    int graySubjs() noexcept override;
  };
}
