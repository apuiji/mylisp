#pragma once

#include<cmath>
#include<concepts>
#include<string>
#include<variant>
#include"myccutils/xyz.hh"

namespace zlt::mylisp {
  struct Object;
  struct Value;

  using NativeFunction = int (Value *it, Value *end);
  using Null = std::monostate;

  struct Value: std::variant<Null, double, const std::wstring *, const std::string *, Object *, NativeFunction *> {
    enum {
      NULL_INDEX, NUM_INDEX, STR_INDEX, LATIN1_INDEX, OBJ_INDEX, NAT_FN_INDEX
    };
    // constructors begin
    using variant::variant;
    Value(std::integral auto i) noexcept: variant((double) i) {}
    Value(bool b) noexcept {
      operator =(b);
    }
    Value(std::derived_from<Object> auto *o) noexcept: variant(static_cast<Object *>(o)) {}
    // constructors end
    // assignment operations begin
    template<AnyOf<Null, double, const std::wstring *, const std::string *, Object *, NativeFunction *> T>
    Value &operator =(T &&t) noexcept {
      variant::operator =(std::forward<T>(t));
      return *this;
    }
    Value &operator =(std::integral auto i) noexcept {
      variant::operator =((double) i);
      return *this;
    }
    Value &operator =(bool b) noexcept {
      if (b) {
        variant::operator =(1.);
      } else {
        variant::operator =(Null());
      }
      return *this;
    }
    Value &operator =(std::derived_from<Object> auto *o) noexcept {
      return operator =(static_cast<Object *>(o));
    }
    // assignment operations end
    // cast operations begin
    operator bool() const noexcept {
      return index() != NULL_INDEX;
    }
    // cast operations end
  };

  // static cast operations begin
  template<AnyOf<double, const std::wstring *, const std::string *, Object *, NativeFunction *, void *> T>
  static inline int staticast(T &dest, const Value &src) noexcept {
    dest = *(T *) &src;
    return 0;
  }

  template<std::derived_from<Object> T>
  static inline int staticast(T *&dest, Value &src) noexcept {
    dest = static_cast<T *>(*(Object **) &src);
    return 0;
  }
  // static cast operations end

  // dynamic cast operations begin
  bool dynamicast(double &dest, const Value &src) noexcept;

  template<std::integral I>
  bool dynamicast(I &dest, const Value &src) noexcept {
    double d;
    if (!dynamicast(d, src)) {
      return false;
    }
    dest = (I) d;
    return true;
  }

  bool dynamicast(std::string_view &dest, const Value &src) noexcept;
  bool dynamicast(std::wstring_view &dest, const Value &src) noexcept;
  bool dynamicast(NativeFunction *&dest, const Value &src) noexcept;
  bool dynamicast(Object *&dest, const Value &src) noexcept;

  template<std::derived_from<Object> T>
  static inline bool dynamicast(T *&dest, const Value &src) noexcept {
    Object *o;
    if (!dynamicast(o, src)) {
      return false;
    }
    dest = dynamic_cast<T *>(o);
    return dest;
  }
  // dynamic cast operations end

  // comparisons begin
  bool operator ==(const Value &a, const Value &b) noexcept;

  static inline bool operator !=(const Value &a, const Value &b) noexcept {
    return !operator ==(a, b);
  }

  bool compare(int &dest, const Value &a, const Value &b) noexcept;

  static inline bool operator <(const Value &a, const Value &b) noexcept {
    int diff;
    return compare(diff, a, b) && diff < 0;
  }

  static inline bool operator >(const Value &a, const Value &b) noexcept {
    int diff;
    return compare(diff, a, b) && diff > 0;
  }

  static inline bool operator <=(const Value &a, const Value &b) noexcept {
    int diff;
    return compare(diff, a, b) && diff <= 0;
  }

  static inline bool operator >=(const Value &a, const Value &b) noexcept {
    int diff;
    return compare(diff, a, b) && diff >= 0;
  }
  // comparisons end

  // member operations begin
  Value getMemb(const Value &v, const Value &memb) noexcept;
  int setMemb(Value &v, const Value &memb, const Value &value);
  // member operations end

  static inline bool operator !(const Value &v) noexcept {
    return v.index() == Value::NULL_INDEX;
  }
}
