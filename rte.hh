#pragma once

#include<set>
#include"coroutine.hh"
#include"myccutils/mymap.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern Coroutines coroutines;
  extern std::map<std::string, Value> mods;
  extern std::set<std::string> fnBodies;
  extern ItCoroutine itCoroutine;
  extern std::set<std::string> strings;

  struct GlobalDefsComp {
    int operator ()(const std::string *a, const std::string *b) const noexcept {
      return a->compare(*b);
    }
  };

  extern mymap::Map<const std::string *, Value, GlobalDefsComp> globalDefs;

  int init();
  int yield();
}
