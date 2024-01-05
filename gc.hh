#pragma once

namespace zlt::mylisp {
  struct Object;
  struct Value;
}

namespace zlt::mylisp::gc {
  int gc() noexcept;
  int grayIt(Value &v) noexcept;
  int neobj(Object *o) noexcept;
  /// insert write barrier
  int iwb(Value &v, Value &w) noexcept;
}
