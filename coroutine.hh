#pragma once

#include<list>
#include"object.hh"

namespace zlt::mylisp {
  struct Coroutine {
    Value ax;
    Value *bp;
    Value *sp;
    const char *pc;
    bool alive;
    Value *valuek;
    Value *valuekEnd;
    Value *deferk;
    Value *deferkEnd;
    char *otherk;
    char *otherkEnd;
    ~Coroutine();
  };

  using Coroutines = std::list<Coroutine>;
  using ItCoroutine = Coroutines::iterator;

  extern Coroutines coroutines;
  extern ItCoroutine itCoroutine;

  namespace it_coroutine {
    static inline Value &ax() noexcept {
      return itCoroutine->ax;
    }

    static inline Value *&bp() noexcept {
      return itCoroutine->bp;
    }

    static inline Value *&sp() noexcept {
      return itCoroutine->sp;
    }

    static inline const char *&pc() noexcept {
      return itCoroutine->pc;
    }

    static inline Value &peek() noexcept {
      return sp()[-1];
    }

    static inline Value pop() noexcept {
      return *--sp();
    }

    static inline FunctionObj *callee() noexcept {
      return staticast<FunctionObj *>(bp()[-1]);
    }

    template<class T>
    static inline T consume() noexcept {
      T t = *(T *) itCoroutine->pc;
      itCoroutine->pc += sizeof(T);
      return t;
    }
  }
}
