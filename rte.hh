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

  using GlobalDef = mymap::Node<const std::string *, Value>;

  extern GlobalDef *globalDefs;

  int globalDefsComp(const std::string *a, const std::string *b) noexcept;

  int init();
  int yield();
}
