#pragma once

#include<deque>
#include<list>
#include"myutils/mymap.hh"
#include"value.hh"

namespace zlt::mylisp {
  struct Object {
    bool mark;
    virtual ~Object() = default;
    virtual bool length(size_t &dest) const noexcept {
      return false;
    }
    virtual std::partial_ordering compare(const Value &v) const noexcept {
      Object *o;
      return mylisp::dynamicast(o, v) && this == o ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
    }
    virtual bool dynamicast(std::string_view &dest) const noexcept {
      return false;
    }
    virtual Value getMemb(const Value &key) const noexcept {
      return Null();
    }
    virtual void setMemb(const Value &key, const Value &value) {
      // do nothing
    }
    virtual void gcMarkSubjs() noexcept {
      // do nothing
    }
  };

  struct ValueObj;

  struct FunctionObj final: Object {
    size_t paramn;
    size_t guardn;
    const char *body;
    size_t closureDefn;
    ValueObj *closureDefs[0];
    FunctionObj(size_t paramn, const char *body, size_t closureDefn) noexcept:
    paramn(paramn), guardn(0), body(body), closureDefn(closureDefn) {}
    void *operator new(size_t size, size_t closureDefn);
    void gcMarkSubjs() noexcept override;
  };

  struct ListObj final: Object {
    std::deque<Value> list;
    ListObj() = default;
    bool length(size_t &dest) const noexcept override;
    Value getMemb(const Value &key) const noexcept override;
    void setMemb(const Value &key, const Value &value) override;
    void gcMarkSubjs() noexcept override;
  };

  struct MapObj final: Object {
    using Map = MyMap<Value, Value>;
    using Node = Map::Node;
    Map map;
    MapObj() = default;
    Value getMemb(const Value &key) const noexcept override;
    void setMemb(const Value &key, const Value &value) override;
    void gcMarkSubjs() noexcept override;
  };

  struct StringObj final: Object {
    std::string value;
    StringObj(const std::string &value): value(value) {}
    StringObj(std::string &&value) noexcept: value(std::move(value)) {}
    bool length(size_t &dest) const noexcept override;
    std::partial_ordering compare(const Value &v) const noexcept override;
    bool dynamicast(std::string_view &dest) const noexcept override;
    Value getMemb(const Value &key) const noexcept override;
  };

  struct StringViewObj final: Object {
    Value string;
    std::string_view value;
    StringViewObj(const Value &string, std::string_view value) noexcept: string(string), value(value) {}
    bool length(size_t &dest) const noexcept override;
    std::partial_ordering compare(const Value &v) const noexcept override;
    bool dynamicast(std::string_view &dest) const noexcept override;
    Value getMemb(const Value &key) const noexcept override;
    void gcMarkSubjs() noexcept override;
  };

  struct ValueObj final: Object {
    Value value;
    void gcMarkSubjs() noexcept override;
  };
}
