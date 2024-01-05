#pragma once

#include<boost/coroutine2/all.hpp>
#include<list>
#include<map>
#include<memory>
#include"value.hh"

namespace zlt::mylisp {
  struct ValueStack {
    size_t size;
    Value data[0];
    void *operator new(size_t n, size_t m);
    Value *begin() noexcept {
      return data;
    }
    Value *end() noexcept {
      return data + size;
    }
    const Value *begin() const noexcept {
      return data;
    }
    const Value *end() const noexcept {
      return data + size;
    }
  };

  struct Coroutine {
    using Sink = boost::coroutines2::coroutine<void>::push_type;
    Value value;
    std::unique_ptr<ValueStack> valuek;
    Value *valuekBottom;
    Value *valuekTop;
    std::list<std::map<const std::wstring *, Value>> localDefsk;
    std::list<Value> deferk;
    bool alive;
    Sink &sink;
    Coroutine(std::unique_ptr<ValueStack> &&valuek, Sink &sink) noexcept: valuek(std::move(valuek)), sink(sink) {}
  };

  using Coroutines = std::list<Coroutine>;
  using ItCoroutine = Coroutines::iterator;
}
