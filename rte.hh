#pragma once

#include<set>
#include"coroutine.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern Coroutines coroutines;
  extern std::set<std::string> fnBodies;
  extern ItCoroutine itCoroutine;
  extern std::set<std::wstring> strings;

  struct GlobalDefsComp {
    bool operator ()(const std::wstring *a, const std::wstring *b) const noexcept;
  };

  extern std::map<const std::wstring *, Value, GlobalDefsComp> globalDefs;

  int init();
  int yield();
}
