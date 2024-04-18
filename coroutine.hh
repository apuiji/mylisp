#pragma once

#include<list>
#include"object.hh"

namespace zlt::mylisp {
  struct Coroutine {
    Value ax;
    Value *bp;
    Value *sp;
    /// defer stack pointer
    Value *dsp;
    /// other stack pointer
    char *osp;
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
    // registers begin
    static inline Value &ax() noexcept {
      return itCoroutine->ax;
    }

    static inline Value *&bp() noexcept {
      return itCoroutine->bp;
    }

    static inline Value *&sp() noexcept {
      return itCoroutine->sp;
    }

    static inline Value *&dsp() noexcept {
      return itCoroutine->dsp;
    }

    static inline char *&osp() noexcept {
      return itCoroutine->osp;
    }

    static inline const char *&pc() noexcept {
      return itCoroutine->pc;
    }
    // registers begin

    // value stack begin
    static inline FunctionObj *callee() noexcept {
      return staticast<FunctionObj *>(bp()[-1]);
    }

    static inline Value &peek() noexcept {
      return sp()[-1];
    }

    static inline Value pop() noexcept {
      return *--sp();
    }

    void push(const Value &value);
    // value stack end

    // defer stack begin
    void pushDefer(const Value &value);
    // defer stack end

    // other stack begin
    template<class T>
    static inline T popOther() noexcept {
      osp() -= sizeof(T);
      return *(T *) osp;
    }

    void pushOther(void *p);

    template<class T>
    requires (!std::is_same_v<T, void>)
    static inline void pushOther(T *p) noexcept {
      pushOther((void *) p);
    }
    // other stack end

    template<class T>
    static inline T consume() noexcept {
      T t = *(T *) itCoroutine->pc;
      itCoroutine->pc += sizeof(T);
      return t;
    }
  }
}
