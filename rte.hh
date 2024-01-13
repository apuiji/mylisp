#pragma once

#include<set>
#include"coroutine.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern Coroutines coroutines;
  extern std::set<std::string> fnBodies;
  extern std::map<const std::wstring *, Value> globalDefs;
  extern ItCoroutine itCoroutine;
  extern std::set<std::wstring> strings;

  int init();
  int yield();
}
