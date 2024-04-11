#pragma once

#include<list>
#include<set>
#include"value.hh"

namespace zlt::mylisp {
  struct Coroutine {
    Value ax;
    Value *bp;
    Value *sp;
    Value *tdsp;
    const char *pc;
    bool alive;
    struct {
      Value *data;
      size_t size;
    } valuek;
    struct {
      char *data;
      size_t size;
    } ctrlk;
  };

  using Coroutines = std::list<Coroutine>;
  using ItCoroutine = Coroutines::iterator;

  extern Coroutines coroutines;
  extern ItCoroutine itCoroutine;
  extern std::set<std::string> bodies;
  extern std::set<std::string> strings;

  const std::string *addString(std::string &&s);
  void exec();
}
