#pragma once

#include<cwchar>
#include<set>
#include"coroutine.hh"
#include"myccutils/mymap.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern Coroutines coroutines;
  extern std::map<std::string, Value> mods;
  extern std::set<std::string> fnBodies;
  extern ItCoroutine itCoroutine;
  extern std::set<std::wstring> strings;

  struct GlobalDefsComp {
    int operator ()(const std::wstring *a, const std::wstring *b) const noexcept {
      return a->compare(*b);
    }
  };

  extern mymap::Map<const std::wstring *, Value, GlobalDefsComp> globalDefs;

  int init();
  int yield();
}
