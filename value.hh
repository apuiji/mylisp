#pragma once

#include<cmath>
#include<concepts>
#include<string>
#include<tuple>
#include<variant>
#include"myccutils/xyz.hh"

namespace zlt::mylisp {
  struct Object;
  struct Value;

  using NativeFunction = Value (const Value *it, const Value *end);
  using Null = std::monostate;

  struct Value: std::variant<Null, double, wchar_t, const std::wstring *, Object *, NativeFunction *> {
    enum {
      NULL_INDEX, NUM_INDEX, CHAR_INDEX, STR_INDEX, OBJ_INDEX, NAT_FN_INDEX
    };
    // constructors begin
    Value() noexcept {}
    Value(Null) noexcept {}
    #define baseCons(T) \
    Value(T t) noexcept: variant(t) {}
    baseCons(double);
    baseCons(wchar_t);
    baseCons(const std::wstring *);
    baseCons(Object *);
    baseCons(NativeFunction *);
    #undef baseCons
    Value(int i) noexcept: variant((double) i) {}
    Value(size_t n) noexcept: variant((double) n) {}
    Value(bool b) noexcept {
      operator =(b);
    }
    Value(std::wstring &&s): Value((std::wstring_view) s) {}
    Value(std::wstring_view sv) {
      operator =(sv);
    }
    Value(const Value &string, std::wstring_view view);
    // constructors end
    // assignment operations begin
    #define baseAssign(T) \
    Value &operator =(T t) noexcept { \
      variant::operator =(t); \
      return *this; \
    }
    baseAssign(Null);
    baseAssign(double);
    baseAssign(wchar_t);
    baseAssign(const std::wstring *);
    baseAssign(Object *);
    baseAssign(NativeFunction *);
    #undef baseAssign
    Value &operator =(int i) noexcept {
      variant::operator =((double) i);
      return *this;
    }
    Value &operator =(size_t n) noexcept {
      variant::operator =((double) n);
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
    Value &operator =(std::wstring &&s) {
      return operator =((std::wstring_view) s);
    }
    Value &operator =(std::wstring_view sv);
    Value &operator =(std::derived_from<Object> auto *o) noexcept {
      variant::operator =(static_cast<Object *>(o));
      return *this;
    }
    // assignment operations end
    // cast operations begin
    operator double() const noexcept {
      auto d = std::get_if<double>(this);
      return d ? *d : NAN;
    }
    operator int() const noexcept {
      return (int) operator double();
    }
    operator bool() const noexcept {
      return index() != NULL_INDEX;
    }
    // cast operations end
  };

  // static cast operations begin
  template<AnyOf<double, wchar_t, const std::wstring *, NativeFunction *, void *> T>
  static inline int staticast(T &dest, const Value &src) noexcept {
    dest = *(T *) &src;
    return 0;
  }

  template<int I>
  static inline int staticast(std::wstring_view &dest, const Value &src) noexcept {
    if constexpr (I == Value::CHAR_INDEX) {
      dest = std::wstring_view((const wchar_t *) &src, 1);
    } else if constexpr (I == Value::STR_INDEX) {
      dest = (std::wstring_view) **(const std::wstring **) &src;
    } else {
      // never
    }
    return 0;
  }

  static inline int staticast(Object *&dest, const Value &src) noexcept {
    dest = *(Object **) &src;
    return 0;
  }

  template<std::derived_from<Object> T>
  static inline int staticast(T *&dest, const Value &src) noexcept {
    dest = static_cast<T *>(*(Object **) &src);
    return 0;
  }
  // static cast operations end

  // dynamic cast operations begin
  #define baseDynamicast(T) \
  static inline bool dynamicast(T &dest, const Value &src) noexcept { \
    if (auto t = std::get_if<T>(&src); t) { \
      dest = *t; \
      return true; \
    } else { \
      return false; \
    } \
  }

  baseDynamicast(double);
  baseDynamicast(wchar_t);
  baseDynamicast(Object *);
  baseDynamicast(NativeFunction *);

  #undef baseDynamicast

  template<class I>
  requires (std::is_integral_v<I> && !std::is_pointer_v<I>)
  bool dynamicast(I &dest, const Value &src) noexcept {
    if (double d; dynamicast(d, src)) {
      dest = (I) d;
      return true;
    } else {
      return false;
    }
  }

  bool dynamicast(std::string_view &dest, const Value &src) noexcept;
  bool dynamicast(std::wstring_view &dest, const Value &src) noexcept;

  template<std::derived_from<Object> T>
  static inline bool dynamicast(T *&dest, const Value &src) noexcept {
    if (Object *o; dynamicast(o, src)) {
      dest = dynamic_cast<T *>(o);
      return dest;
    } else {
      return false;
    }
  }

  template<class T>
  static inline bool dynamicast(T &dest, const Value *it, const Value *end) noexcept {
    return it < end && dynamicast(dest, *it);
  }

  template<int I = 0, class ...T>
  static inline bool dynamicasts(const std::tuple<T *...> &dest, const Value *it, const Value *end) noexcept {
    if constexpr (I == sizeof...(T)) {
      return true;
    } else {
      return dynamicast(*std::get<I>(dest), it, end) && dynamicasts<I + 1>(dest, it + 1, end);
    }
  }
  // dynamic cast operations end

  // comparisons begin
  #define compBetween(T) \
  bool operator ==(const Value &a, T b) noexcept; \
  \
  static inline bool operator !=(const Value &a, T b) noexcept { \
    return !operator ==(a, b); \
  } \
  \
  bool compare(int &dest, const Value &a, T b) noexcept; \
  \
  static inline bool operator <(const Value &a, T b) noexcept { \
    int diff; \
    return compare(diff, a, b) && diff < 0; \
  } \
  \
  static inline bool operator >(const Value &a, T b) noexcept { \
    int diff; \
    return compare(diff, a, b) && diff > 0; \
  } \
  \
  static inline bool operator <=(const Value &a, T b) noexcept { \
    int diff; \
    return compare(diff, a, b) && diff <= 0; \
  } \
  \
  static inline bool operator >=(const Value &a, T b) noexcept { \
    int diff; \
    return compare(diff, a, b) && diff >= 0; \
  }

  compBetween(const Value &);
  compBetween(double);
  compBetween(std::wstring_view);

  #undef compBetween

  #define compBetween1(T) \
  static inline bool operator ==(T a, const Value &b) noexcept { \
    return b == a; \
  } \
  \
  static inline bool operator !=(T a, const Value &b) noexcept { \
    return b != a; \
  } \
  \
  static inline bool operator <(T a, const Value &b) noexcept { \
    int diff; \
    return compare(diff, b, a) && diff > 0; \
  } \
  \
  static inline bool operator >(T a, const Value &b) noexcept { \
    int diff; \
    return compare(diff, b, a) && diff < 0; \
  } \
  \
  static inline bool operator <=(T a, const Value &b) noexcept { \
    int diff; \
    return compare(diff, b, a) && diff >= 0; \
  } \
  \
  static inline bool operator >=(T a, const Value &b) noexcept { \
    int diff; \
    return compare(diff, b, a) && diff <= 0; \
  }

  compBetween1(double);
  compBetween1(std::wstring_view);

  #undef compBetween1
  // comparisons end

  // member operations begin
  Value getMemb(const Value &v, const Value &memb) noexcept;
  int setMemb(Value &v, const Value &memb, const Value &value);
  // member operations end

  static inline bool operator !(const Value &v) noexcept {
    return v.index() == Value::NULL_INDEX;
  }

  // const string * begin
  template<int ...S>
  struct Constring {
    static const std::wstring value;
  };

  template<int ...S>
  const std::wstring Constring<S...>::value = { S... };

  template<int ...S>
  static inline const std::wstring *constring = &Constring<S...>::value;
  // const string * end
}
