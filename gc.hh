#pragma once

namespace zlt::mylisp {
  struct Object;
  struct Value;
}

namespace zlt::mylisp::gc {
  int gc() noexcept;
  int grayObj(Object *o) noexcept;
  int grayValue(const Value &v) noexcept;
  /// insert write barrier
  int iwb(Value &v, Value &w) noexcept;
  int neobj(Object *o) noexcept;
}
