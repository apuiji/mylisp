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
    virtual bool length(size_t &dest) const noexcept = 0;
    virtual Value getMemb() const noexcept = 0;
    virtual void setMemb(const Value &key, const Value &value) = 0;
  };

  struct FunctionObj final: Object {
    size_t paramn;
    size_t defern = 0;
    const char *body;
    Value closureDefs[0];
    FunctionObj(size_t paramn, const char *body) noexcept: paramn(paramn), body(body) {}
    void *operator new(size_t size, size_t closureDefn);
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
