#pragma once

namespace zlt::mylisp {
  struct Object;
  struct Value;
}

namespace zlt::mylisp::gc {
  enum {
    WHITE_COLOR,
    GRAY_COLOR,
    BLACK_COLOR
  };

  struct Head {
    Object *prev;
    Object *next;
    int color = BLACK_COLOR;
  };

  extern Object *blacks;

  int gc() noexcept;
  int grayObj(Object *o) noexcept;
  int grayValue(const Value &v) noexcept;
  int put(Object *&dest, Object *o) noexcept;
}
