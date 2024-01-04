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
    const char *next;
    const char *end;
    Frame(const char *next, const char *end) noexcept: next(next), end(end) {}
  };

  struct CallFrame: Frame {
    Value *prevValuekBottom;
    size_t prevDeferkSize;
    CallFrame(const char *next, const char *end, Value *prevValuekBottom, size_t prevDeferkSize) noexcept:
    Frame(next, end), prevValuekBottom(prevValuekBottom), prevDeferkSize(prevDeferkSize) {}
  };

  struct DeferFrame {
    size_t deferc;
    DeferFrame(size_t deferc) noexcept;
  };

  struct ForwardFrame {
    size_t argc;
    ForwardFrame(size_t argc) noexcept;
  };

  struct TryFrame: Frame {
    size_t prevDeferkSize;
    TryFrame(const char *next, const char *end, size_t prevDeferkSize) noexcept:
    Frame(next, end), prevDeferkSize(prevDeferkSize) {}
  };

  struct VarFrame: std::variant<CallFrame, DeferFrame, ForwardFrame, TryFrame> {
    template<class T>
    operator T *() noexcept {
      return (T *) this;
    }
    Frame *operator ->() noexcept {
      return (Frame *) this;
    }
  };

  struct Coroutine {
    Value value;
    std::unique_ptr<Value> valuek;
    Value *valuekBottom;
    Value *valuekTop;
    std::list<std::map<const std::wstring *, Value>> localDefsk;
    std::list<Value> deferk;
    std::list<VarFrame> framek;
  };
}
