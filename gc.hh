#pragma once

namespace zlt::mylisp {
  struct Object;
  struct Value;
}

namespace zlt::mylisp::gc {
  void grayObj(Object *obj) noexcept;
  void grayValue(Value &value) noexcept;
  void neobj(Object *obj) noexcept;
}
