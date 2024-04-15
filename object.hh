#pragma once

#include"value.hh"

namespace zlt::mylisp {
  struct Object {
    enum {
      BLACK_COLOR,
      GRAY_COLOR,
      WHITE_COLOR
    };
    int color;
    Object *prev;
    Object *next;
    virtual ~Object() = default;
  };

  struct FunctionObj final: Object {
    size_t paramn;
    size_t defern = 0;
    const char *body;
    Value closures[0];
    FunctionObj(size_t paramn, const char *body) noexcept: paramn(paramn), body(body) {}
    void *operator new(size_t size, size_t closuren);
  };

  struct StringObj final: Object {
    std::string value;
    StringObj(const std::string &value): value(value) {}
    StringObj(std::string &&value) noexcept: value(std::move(value)) {}
  };

  struct StringViewObj final: Object {
    Value string;
    std::string_view value;
    StringViewObj(const Value &string, std::string_view) noexcept: string(string), value(value) {}
  };
}
