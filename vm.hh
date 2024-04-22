#pragma once

#include<list>
#include"object.hh"

namespace zlt::mylisp::vm {
  // registers begin
  extern Value ax;
  // base pointer
  extern Value *bp;
  // stack pointer
  extern Value *sp;
  /// guard stack pointer
  extern Value *gsp;
  /// defer stack pointer
  extern Value *dsp;
  /// other stack pointer
  extern char *osp;
  extern const char *pc;
  // registers end

  namespace valuek {
    extern Value *begin;
    extern Value *end;

    static inline Value &peek() noexcept {
      return sp[-1];
    }

    static inline Value pop() noexcept {
      return *--sp;
    }

    void push(const Value &value);
  }

  static inline FunctionObj *callee() noexcept {
    return staticast<FunctionObj *>(bp[-1]);
  }

  namespace guardk {
    extern Value *begin;
    extern Value *end;

    static inline Value &peek() noexcept {
      return gsp[-1];
    }

    static inline Value pop() noexcept {
      return *--gsp;
    }

    void push(const Value &value);
  }

  namespace deferk {
    extern Value *begin;
    extern Value *end;

    static inline Value &peek() noexcept {
      return dsp[-1];
    }

    static inline Value pop() noexcept {
      return *--dsp;
    }

    void push(const Value &value);
  }

  namespace otherk {
    extern char *begin;
    extern char *end;

    template<class T>
    static inline T pop() noexcept {
      osp -= sizeof(T);
      return *(T *) osp;
    }

    void push(void *p, size_t size);

    template<class T>
    static inline void push(T &&t) noexcept {
      push(&t, sizeof(T));
    }
  }

  template<class T>
  static inline T consume() noexcept {
    T t = *(T *) itCoroutine->pc;
    itCoroutine->pc += sizeof(T);
    return t;
  }
}
