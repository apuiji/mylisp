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
    // using variant::variant;
    // Value(std::integral auto i) noexcept: variant((double) i) {}
    // Value(bool b) noexcept {
    //   operator =(b);
    // }
    // Value(std::true_type) noexcept: variant(1.) {}
    // Value(std::false_type) noexcept {}
    // Value(std::derived_from<Object> auto *o) noexcept: variant(static_cast<Object *>(o)) {}
    // // constructors end
    // // assignment operations begin
    // template<class T>
    // Value &operator =(T &&t) noexcept {
    //   variant::operator =(std::forward<T>(t));
    //   return *this;
    // }
    // Value &operator =(std::integral auto i) noexcept {
    //   variant::operator =((double) i);
    //   return *this;
    // }
    Value &operator =(bool b) noexcept {
      return b ? operator =(true) : operator =(false);
    }
    Value &operator =(std::true_type) noexcept {
      variant::operator =(1.);
      return *this;
    }
    Value &operator =(std::false_type) noexcept {
      variant::operator =(Null());
      return *this;
    }
    // Value &operator =(Object *o) noexcept {
    //   variant::operator =(o);
    //   return *this;
    // }
    // Value &operator =(std::derived_from<Object> auto *o) noexcept {
    //   return operator =(static_cast<Object *>(o));
    // }
    // // assignment operations end
    // // cast operations begin
    // operator double() const noexcept;
    // template<std::integral I>
    // operator I() const noexcept {
    //   return (I) operator double();
    // }
    operator bool() const noexcept {
      return index() != NULL_INDEX;
    }
    // operator Object *() const noexcept;
    // template<std::derived_from<Object> T>
    // operator T *() const noexcept {
    //   return static_cast<T *>(operator Object *());
    // }
    // // cast operations end
    Value &operator [](const Value &key) noexcept;
    // const Value &operator [](const Value &key) const noexcept;
  };

  template<AnyOf<double, const std::wstring *, const std::string *, Object *, NativeFunction *, void *> T>
  static inline int staticast(T &dest, const Value &src) noexcept {
    dest = *(T *) &src;
    return 0;
  }

  int staticast(std::string_view &dest, const Value &src) noexcept;
  int staticast(std::wstring_view &dest, const Value &src) noexcept;

  template<std::derived_from<Object> T>
  static inline int staticast(T *&dest, Value &src) noexcept {
    dest = static_cast<T *>(*(Object **) &src);
    return 0;
  }

  bool dynamicast(double &dest, const Value &src) noexcept;
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

  static inline bool operator !(const Value &v) noexcept {
    return v.index() == Value::NULL_INDEX;
  }

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
}
