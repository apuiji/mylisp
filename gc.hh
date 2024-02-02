#pragma once

#include"value.hh"

namespace zlt::mylisp::gc {
  extern Object *blacks;

  int gc() noexcept;
  int grayObj(Object *o) noexcept;
  int grayValue(const Value &v) noexcept;
  int put(Object *&dest, Object *o) noexcept;

  // insert write barrier begin
  /// @param o not null
  /// @param p not null
  int iwb(Object *o, Object *p) noexcept;
  /// @param o not null
  int iwb(Object *o, const Value &v) noexcept;

  static inline int iwb(const Value &v, const Value &w) noexcept {
    if (Object *o; dynamicast(o, v)) {
      iwb(o, w);
    }
    return 0;
  }
  // insert write barrier end

  // delete write barrier begin
  /// @param o not null
  /// @param p not null
  int dwb(Object *o, Object *p) noexcept;
  /// @param o not null
  int dwb(Object *o, const Value &v) noexcept;

  static inline int dwb(const Value &v, const Value &w) noexcept {
    if (Object *o; dynamicast(o, v)) {
      dwb(o, w);
    }
    return 0;
  }
  // delete write barrier end
}
