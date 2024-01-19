#pragma once

#include<cwchar>
#include<set>
#include"coroutine.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  struct GlobalDefsKeyComparator {
    bool operator ()(const wchar_t *a, const wchar_t *b) const noexcept {
      return std::wcscmp(a, b) < 0;
    }
  };

  extern Coroutines coroutines;
  extern std::set<std::string> fnBodies;
  extern std::map<const wchar_t *, Value, GlobalDefsKeyComparator> globalDefs;
  extern ItCoroutine itCoroutine;
  extern std::set<std::wstring> strings;

  int init();
  int yield();
}
