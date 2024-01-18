#pragma once

#include<deque>
#include<map>
#include<set>
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
    virtual bool objDynamicast(std::wstring_view &dest) const noexcept {
      return false;
    }
    virtual bool objDynamicast(std::string_view &dest) const noexcept {
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
    virtual Value objGetMemb(const Value &memb) const noexcept {
      return Null();
    }
    virtual int objSetMemb(const Value &memb, const Value &value) {
      return 0;
    }
    // member operations end
    virtual int graySubjs() noexcept {
      return 0;
    }
  };

  // comparisons begin
  static inline bool operator !=(const Object &o, const Value &v) noexcept {
    return !(o == v);
  }

  static inline bool operator <(const Object &o, const Value &v) noexcept {
    int diff;
    return o.compare(diff, v) && diff < 0;
  }

  static inline bool operator >(const Object &o, const Value &v) noexcept {
    int diff;
    return o.compare(diff, v) && diff > 0;
  }

  static inline bool operator <=(const Object &o, const Value &v) noexcept {
    int diff;
    return o.compare(diff, v) && diff <= 0;
  }

  static inline bool operator >=(const Object &o, const Value &v) noexcept {
    int diff;
    return o.compare(diff, v) && diff >= 0;
  }

  static inline bool operator ==(const Value &v, const Object &o) noexcept {
    return o == v;
  }

  static inline bool operator !=(const Value &v, const Object &o) noexcept {
    return o != v;
  }

  static inline bool compare(int &dest, const Value &v, const Object &o) noexcept {
    if (o.compare(dest, v)) {
      dest = -dest;
      return true;
    } else {
      return false;
    }
  }

  static inline bool operator <(const Value &v, const Object &o) noexcept {
    int diff;
    return compare(diff, v, o) && diff < 0;
  }

  static inline bool operator >(const Value &v, const Object &o) noexcept {
    int diff;
    return compare(diff, v, o) && diff > 0;
  }

  static inline bool operator <=(const Value &v, const Object &o) noexcept {
    int diff;
    return compare(diff, v, o) && diff <= 0;
  }

  static inline bool operator >=(const Value &v, const Object &o) noexcept {
    int diff;
    return compare(diff, v, o) && diff >= 0;
  }
  // comparisons end

  template<class C>
  struct BasicStringViewObj: virtual Object {
    // cast operations begin
    virtual operator std::basic_string_view<C>() const noexcept = 0;
    bool objDynamicast(std::basic_string_view<C> &dest) const noexcept override {
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
    Value objGetMemb(const Value &memb) const noexcept override;
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
    int graySubjs() noexcept override {
      gc::grayValue(string);
      return 0;
    }
    operator std::basic_string_view<C>() const noexcept override {
      return view;
    }
  };

  using StringViewObj = BasicStringViewObj1<wchar_t>;
  using Latin1ViewObj = BasicStringViewObj1<char>;

  template<class C>
  Value BasicStringViewObj<C>::objGetMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    auto sv = operator std::basic_string_view<C>();
    if (!(i >= 0 && i < sv.size())) {
      return Null();
    }
    return sv[i];
  }

  struct FunctionObj final: Object {
    std::map<const std::wstring *, Value> closures;
    const std::string &body;
    FunctionObj(const std::string &body) noexcept: body(body) {}
    int graySubjs() noexcept override;
  };

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

  struct MapObj final: Object {
    struct StrPoolComparator {
      bool operator ()(const Value &a, const Value &b) const noexcept;
    };
    std::pair<bool, Value> nullPool;
    std::pair<bool, Value> nanPool;
    std::map<double, Value> numPool;
    std::map<wchar_t, Value> charPool;
    std::map<Value, Value, StrPoolComparator> strPool;
    std::map<Object *, Value> objPool;
    std::map<void *, Value> ptrPool;
    // member operations begin
    Value objGetMemb(const Value &memb) const noexcept override;
    int objSetMemb(const Value &memb, const Value &value) override;
    // member operations end
    int graySubjs() noexcept override;
  };

  struct PointerObj final: Object {
    Value value;
    int graySubjs() noexcept override;
  };

  static inline Value &operator *(PointerObj &p) noexcept {
    return p.value;
  }

  static inline const Value &operator *(const PointerObj &p) noexcept {
    return p.value;
  }
}
