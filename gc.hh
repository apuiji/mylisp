#pragma once

namespace zlt::mylisp {
  struct Object;
}

namespace zlt::mylisp::gc {
  int neobj(Object *o) noexcept;
}
