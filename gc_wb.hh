#pragma once

#include"object.hh"

namespace zlt::mylisp::gc {
  // insert write barrier begin
  /// @param o not null
  /// @param p not null
  static inline int iwb(Object *o, Object *p) noexcept {
    if (o->color == BLACK_COLOR) {
      grayObj(p);
    }
    return 0;
  }

  /// @param o not null
  static inline int iwb(Object *o, const Value &v) noexcept {
    if (o->color == BLACK_COLOR) {
      grayValue(v);
    }
    return 0;
  }

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
  static inline int dwb(Object *o, Object *p) noexcept {
    if (o->color != BLACK_COLOR) {
      grayObj(p);
    }
    return 0;
  }

  /// @param o not null
  static inline int dwb(Object *o, const Value &v) noexcept {
    if (o->color != BLACK_COLOR) {
      grayValue(v);
    }
    return 0;
  }

  static inline int dwb(const Value &v, const Value &w) noexcept {
    if (Object *o; dynamicast(o, v)) {
      dwb(o, w);
    }
    return 0;
  }
  // delete write barrier end
}
