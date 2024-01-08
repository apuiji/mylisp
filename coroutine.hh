#pragma once

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

  struct Frame {
    enum {
      CALL_FN_FRAME_CLASS,
      CONTINUE_FORWARD_FRAME_CLASS,
      CONTINUE_RETURN_FRAME_CLASS,
      CONTINUE_THROW_FRAME_CLASS,
      DEFER_FRAME_CLASS,
      TRY_FRAME_CLASS
    };
    int clazz;
    const char *next;
    const char *end;
    Value *valuekBottom;
    Value *valuekTop;
    Frame(int clazz, const char *next, const char *end, Value *valuekBottom, Value *valuekTop) noexcept:
    clazz(clazz), next(next), end(end), valuekBottom(valuekBottom), valuekTop(valuekTop) {}
  };

  struct Coroutine {
    Value value;
    std::unique_ptr<ValueStack> valuek;
    Value *valuekBottom;
    Value *valuekTop;
    std::list<std::map<const std::wstring *, Value>> localDefsk;
    std::list<Value> deferk;
    std::list<Frame> framek;
    bool alive;
    Coroutine(std::unique_ptr<ValueStack> &&valuek) noexcept: valuek(std::move(valuek)) {}
  };

  using Coroutines = std::list<Coroutine>;
  using ItCoroutine = Coroutines::iterator;
}
