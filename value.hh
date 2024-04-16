#pragma once

#include<cmath>
#include<compare>
#include<concepts>
#include<variant>
#include"myutils/xyz.hh"

namespace zlt::mylisp {
  struct Object;
  struct Value;

  using NativeFunction = void (Value &dest, const Value *it, const Value *end);
  using Null = std::monostate;

  struct Value {
    enum {
      NULL_INDEX,
      NUM_INDEX,
      CHAR_INDEX,
      STR_INDEX,
      OBJ_INDEX,
      NAT_FN_INDEX
    };
    using Var = std::variant<Null, double, char, const std::string *, Object *, NativeFunction *>;
    Var var;
    // constructors begin
    Value() = default;
    template<class T>
    Value(T &&t) noexcept: var(std::forward<T>(t)) {}
    Value(std::integral auto i) noexcept: var((double) i) {}
    Value(bool b) noexcept {
      if (b) {
        var = 1.;
      }
    }
    Value(std::partial_ordering o) noexcept {
      using O = std::partial_ordering;
      var = o == O::equivalent ? 0 : o == O::less ? -1 : o == O::greater ? 1 : NAN;
    }
    Value(std::derived_from<Object> auto *o) noexcept: var(static_cast<Object *>(o)) {}
    // constructors end
    // assigners begin
    template<class T>
    Value &operator =(T &&t) noexcept {
      var = std::forward<T>(t);
      return *this;
    }
    Value &operator =(std::integral auto i) noexcept {
      var = (double) i;
      return *this;
    }
    Value &operator =(bool b) noexcept {
      if (b) {
        var = 1.;
      } else {
        var = Null();
      }
      return *this;
    }
    Value &operator =(std::partial_ordering o) noexcept {
      using O = std::partial_ordering;
      var = o == O::equivalent ? 0 : o == O::less ? -1 : o == O::greater ? 1 : NAN;
      return *this;
    }
    Value &operator =(std::derived_from<Object> auto *o) noexcept {
      var = static_cast<Object *>(o);
      return *this;
    }
    // assigners end
    // cast begin
    operator double() const noexcept {
      auto d = get_if<double>(&var);
      return d ? *d : NAN;
    }
    template<std::integral I>
    operator I() const noexcept {
      return (I) operator double();
    }
    operator bool() const noexcept {
      return var.index() != NULL_INDEX;
    }
    // cast end
  };

  // cast begin
  template<AnyOf<double, char, const std::string *, Object *, NativeFunction *> T>
  static inline bool dynamicast(T &dest, const Value &value) noexcept {
    if (T *t = get_if<T>(&value.var); t) {
      dest = *t;
      return true;
    } else {
      return false;
    }
  }

  template<std::integral I>
  static inline bool dynamicast(I &dest, const Value &value) noexcept {
    if (double d; dynamicast(d, value)) {
      dest = (I) d;
      return true;
    } else {
      return false;
    }
  }

  template<std::derived_from<Object> T>
  static inline bool dynamicast(T *&dest, const Value &value) noexcept {
    if (Object *o; dynamicast(o, value)) {
      dest = dynamic_cast<T *>(o);
      return dest != nullptr;
    } else {
      return false;
    }
  }

  template<AnyOf<double, char, const std::string *, Object *, NativeFunction *> T>
  static inline auto &staticast(const Value &value) noexcept {
    return *(T *) &value.var;
  }

  template<std::integral I>
  static inline auto staticast(const Value &value) noexcept {
    return (I) staticast<double>(value);
  }

  template<std::derived_from<Object> T>
  static inline auto staticast(const Value &value) noexcept {
    return static_cast<T *>(staticast<Object *>(value));
  }
  // cast end

  // comparisons begin
  std::partial_ordering operator <=>(const Value &a, const Value &b) noexcept;

  static inline bool operator ==(const Value &a, const Value &b) noexcept {
    return a <=> b == std::partial_ordering::equivalent;
  }

  static inline bool operator !=(const Value &a, const Value &b) noexcept {
    return a <=> b != std::partial_ordering::equivalent;
  }

  static inline bool operator <(const Value &a, const Value &b) noexcept {
    return a <=> b == std::partial_ordering::less;
  }

  static inline bool operator >(const Value &a, const Value &b) noexcept {
    return a <=> b == std::partial_ordering::greater;
  }

  static inline bool operator <=(const Value &a, const Value &b) noexcept {
    auto c = a <=> b;
    return c == std::partial_ordering::less || c == std::partial_ordering::equivalent;
  }

  static inline bool operator >=(const Value &a, const Value &b) noexcept {
    auto c = a <=> b;
    return c == std::partial_ordering::greater || c == std::partial_ordering::equivalent;
  }
  // comparisons end

  bool length(size_t &dest, const Value &src) noexcept;
  Value getMemb(const Value &cont, const Value &key) const noexcept;
  void setMemb(Value &cont, const Value &key, const Value &value);
}
