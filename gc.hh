#pragma once

namespace zlt::mylisp {
  struct Object;
}

namespace zlt::mylisp::gc {
  int gc() noexcept;
  int grayIt(Object *o) noexcept;
  int neobj(Object *o) noexcept;
}
