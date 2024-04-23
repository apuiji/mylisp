#pragma once

namespace zlt::mylisp {
  struct Object;
  struct Value;
}

namespace zlt::mylisp::gc {
  void gc(const Value *args, size_t argc) noexcept;
  void mark(Object *obj) noexcept;
  void mark(Value &value) noexcept;
}
