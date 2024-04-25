#pragma once

#include<list>
#include"value.hh"

namespace zlt::mylisp {
  struct Object {
    bool mark;
    virtual ~Object() = default;
    virtual bool length(size_t &dest) const noexcept {
      return false;
    }
    virtual Value getMemb() const noexcept {
      return Null();
    }
    virtual void setMemb(const Value &key, const Value &value) {
      // do nothing
    }
    virtual void gcMarkSubjs() noexcept {
      // do nothing
    }
  };

  struct FunctionObj final: Object {
    size_t paramn;
    size_t guardn;
    const char *body;
    Value closureDefs[0];
    FunctionObj(size_t paramn, const char *body) noexcept: paramn(paramn), guardn(0), body(body) {}
    void *operator new(size_t size, size_t closureDefn);
    void gcMarkSubjs() noexcept override;
  };

  struct StringObj final: Object {
    std::string value;
    StringObj(const std::string &value): value(value) {}
    StringObj(std::string &&value) noexcept: value(std::move(value)) {}
    bool length(size_t &dest) const noexcept override;
    Value getMemb() const noexcept override;
  };

  struct StringViewObj final: Object {
    Value string;
    std::string_view value;
    StringViewObj(const Value &string, std::string_view value) noexcept: string(string), value(value) {}
    bool length(size_t &dest) const noexcept override;
    Value getMemb() const noexcept override;
    void gcMarkSubjs() noexcept override;
  };

  struct ValueObj final: Object {
    Value value;
    void gcMarkSubjs() noexcept override;
  };
}
