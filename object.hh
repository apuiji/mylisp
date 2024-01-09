#pragma once

#include<deque>
#include<map>
#include"gc.hh"
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
    // cast operations begin
    virtual bool dynamicast(std::wstring_view &dest) const noexcept {
      return false;
    }
    virtual bool dynamicast(std::string_view &dest) const noexcept {
      return false;
    }
    // cast operations end
    // comparisons begin
    virtual bool operator ==(const Value &v) const noexcept {
      auto o = std::get_if<Object *>(&v);
      return o && this == *o;
    }
    /// @param[out] dest comparison result
    /// @return comparable
    virtual bool compare(int &dest, const Value &v) const noexcept {
      auto o = std::get_if<Object *>(&v);
      if (o && this == *o) {
        dest = 0;
        return true;
      } else {
        return false;
      }
    }
    // comparisons end
    // member operations begin
    virtual Value getMemb(const Value &memb) const noexcept {
      return Null();
    }
    virtual int setMemb(const Value &memb, const Value &value) noexcept {
      return 0;
    }
    // member operations end
    virtual int graySubjs() noexcept {
      return 0;
    }
  };

  template<class C>
  struct BasicStringViewObj: virtual Object {
    // cast operations begin
    virtual operator std::basic_string_view<C>() const noexcept = 0;
    bool dynamicast(std::basic_string_view<C> &dest) const noexcept override {
      dest = operator std::basic_string_view<C>();
      return true;
    }
    // cast operations end
    // comparisons begin
    bool operator ==(const Value &v) const noexcept override {
      std::basic_string_view<C> sv;
      return dynamicast(sv, v) && operator std::basic_string_view<C>() == sv;
    }
    bool compare(int &dest, const Value &v) const noexcept override {
      std::basic_string_view<C> sv;
      if (dynamicast(sv, v)) {
        dest = operator std::basic_string_view<C>().compare(sv);
        return true;
      } else {
        return false;
      }
    }
    // comparisons end
    // member operations begin
    Value getMemb(const Value &memb) const noexcept override;
    // member operations end
  };

  template<class C>
  struct BasicStringObj final: BasicStringViewObj<C> {
    std::basic_string<C> value;
    BasicStringObj(const std::basic_string<C> &value) noexcept: value(value) {}
    BasicStringObj(std::basic_string<C> &&value) noexcept: value(std::move(value)) {}
    operator std::basic_string_view<C>() const noexcept override {
      return (std::basic_string_view<C>) value;
    }
  };

  using StringObj = BasicStringObj<wchar_t>;
  using Latin1Obj = BasicStringObj<char>;

  template<class C>
  struct BasicStringViewObj1 final: BasicStringViewObj<C> {
    Value string;
    std::basic_string_view<C> view;
    BasicStringViewObj1(const Value &string, std::basic_string_view<C> view) noexcept: string(string), view(view) {}
    int graySubjs() noexcept override;
    operator std::basic_string_view<C>() const noexcept override {
      return view;
    }
  };

  using StringViewObj = BasicStringViewObj1<wchar_t>;
  using Latin1ViewObj = BasicStringViewObj1<char>;

  template<class C>
  Value BasicStringViewObj<C>::getMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    auto sv = operator std::basic_string_view<C>();
    if (!(i >= 0 && i < sv.size())) {
      return Null();
    }
    auto so = new BasicStringObj<C>(basic_string<C>(1, sv[i]));
    gc::neobj(so);
    return so;
  }

  struct FunctionObj final: Object {
    std::map<const std::wstring *, Value> closures;
    const std::string &body;
    FunctionObj(const std::string &body) noexcept: body(body) {}
    int graySubjs() noexcept override;
  };

  struct ListObj final: Object {
    std::deque<Value> list;
    // member operations begin
    Value getMemb(const Value &memb) const noexcept override;
    int setMemb(const Value &memb, const Value &value) noexcept override;
    // member operations end
    int graySubjs() noexcept override;
  };

  struct ListViewObj final: Object {
    Value list;
    int beginIndex;
    int endIndex;
    ListViewObj(const Value &list, int beginIndex, int endIndex) noexcept:
    list(list), beginIndex(beginIndex), endIndex(endIndex) {}
    // member operations begin
    Value getMemb(const Value &memb) const noexcept override;
    int setMemb(const Value &memb, const Value &value) noexcept override;
    // member operations end
    int graySubjs() noexcept override;
  };

  struct MapObj final: Object {
    struct KeyComparator {
      bool operator ()(const Value &a, const Value &b) const noexcept;
    };
    std::map<Value, Value, KeyComparator> map;
    // member operations begin
    Value getMemb(const Value &memb) const noexcept override {
      return map.find(memb)->second;
    }
    int setMemb(const Value &memb, const Value &value) noexcept override {
      map[memb] = value;
      return 0;
    }
    // member operations end
    int graySubjs() noexcept override;
  };

  struct PointerObj final: Object {
    Value value;
    int graySubjs() noexcept override;
    bool compare(int &dest, const Value &v) const noexcept override;
  };

  static inline Value &operator *(PointerObj &p) noexcept {
    return p.value;
  }

  static inline const Value &operator *(const PointerObj &p) noexcept {
    return p.value;
  }
}
