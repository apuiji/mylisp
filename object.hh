#pragma once

#include<deque>
#include<map>
#include<set>
#include"gc.hh"
#include"myccutils/mymap.hh"
#include"value.hh"

namespace zlt::mylisp {
  struct Object: gc::Head {
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
    virtual bool operator ==(std::string_view s) const noexcept {
      return false;
    }
    virtual bool operator ==(std::wstring_view s) const noexcept {
      return false;
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
    virtual bool compare(int &dest, std::string_view) const noexcept {
      return false;
    }
    virtual bool compare(int &dest, std::wstring_view) const noexcept {
      return false;
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

  template<std::derived_from<Object> T, class ...Args>
  static inline T *neobj(Args &&...args) {
    T *t = new T(std::forward<Args>(args)...);
    gc::put(gc::blacks, t);
    return t;
  }

  // comparisons begin
  #define compBetween(T) \
  static inline bool operator !=(const Object &o, T t) noexcept { \
    return !(o == t); \
  } \
  \
  static inline bool operator <(const Object &o, T t) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff < 0; \
  } \
  \
  static inline bool operator >(const Object &o, T t) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff > 0; \
  } \
  \
  static inline bool operator <=(const Object &o, T t) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff <= 0; \
  } \
  \
  static inline bool operator >=(const Object &o, T t) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff >= 0; \
  } \
  \
  static inline bool operator ==(T t, const Object &o) noexcept { \
    return o == t; \
  } \
  \
  static inline bool operator !=(T t, const Object &o) noexcept { \
    return o != t; \
  } \
  \
  static inline bool compare(int &dest, T t, const Object &o) noexcept { \
    if (o.compare(dest, t)) { \
      dest = -dest; \
      return true; \
    } else { \
      return false; \
    } \
  } \
  \
  static inline bool operator <(T t, const Object &o) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff > 0; \
  } \
  \
  static inline bool operator >(T t, const Object &o) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff < 0; \
  } \
  \
  static inline bool operator <=(T t, const Object &o) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff >= 0; \
  } \
  \
  static inline bool operator >=(T t, const Object &o) noexcept { \
    int diff; \
    return o.compare(diff, t) && diff <= 0; \
  }

  compBetween(const Value &);
  compBetween(std::string_view);
  compBetween(std::wstring_view);

  #undef compBetween
  // comparisons end

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
    struct StrPoolComp {
      int operator ()(std::wstring_view x, const Value &b) const noexcept {
        std::wstring_view y;
        dynamicast(y, b);
        return x.compare(y);
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
