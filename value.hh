#pragma once

#include<cmath>
#include<concepts>
#include<string>
#include<variant>

namespace zlt::mylisp {
  struct Object;
  struct Value;

  using NativeFunction = int (Value *it, Value *end);
  using Null = std::monostate;

  struct Value: std::variant<Null, double, wchar_t, const std::wstring *, const std::string *, Object *, NativeFunction *> {
    // constructors begin
    using variant::variant;
    Value(std::integral auto i) noexcept: variant((double) i) {}
    Value(bool b) noexcept {
      operator =(b);
    }
    Value(std::true_type) noexcept: variant(1.) {}
    Value(std::false_type) noexcept {}
    Value(std::derived_from<Object> auto *o) noexcept: variant(static_cast<Object *>(o)) {}
    // constructors end
    // assignment operations begin
    template<class T>
    Value &operator =(T &&t) noexcept {
      variant::operator =(std::forward<T>(t));
      return *this;
    }
    Value &operator =(std::integral auto i) noexcept {
      variant::operator =((double) i);
      return *this;
    }
    Value &operator =(bool b) noexcept {
      return b ? operator =(true) : operator =(false);
    }
    Value &operator =(std::true_type) noexcept {
      return operator =(1.);
    }
    Value &operator =(std::false_type) noexcept {
      return operator =(Null());
    }
    Value &operator =(Object *o) noexcept {
      variant::operator =(o);
      return *this;
    }
    Value &operator =(std::derived_from<Object> auto *o) noexcept {
      return operator =(static_cast<Object *>(o));
    }
    // assignment operations end
    operator double() const noexcept;
    template<std::integral I>
    operator I() const noexcept;
    operator bool() const noexcept;
  };

  static inline bool operator !(const Value &v) noexcept {
    return !(bool) v;
  }

  bool operator ==(const Value &a, const Value &b) noexcept;

  static inline bool operator !=(const Value &a, const Value &b) noexcept {
    return !operator ==(a, b);
  }

  bool compare(int &dest, const Value &a, const Value &b) noexcept;
}
