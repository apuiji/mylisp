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
  extern std::map<std::string, void *> dlibs;
  extern std::set<std::string> fnBodies;
<<<<<<< HEAD
=======
  extern std::map<const wchar_t *, Value, GlobalDefsKeyComparator> globalDefs;
>>>>>>> e73a35fe0e4ccd47611f22acb4c9e57a00823bfa
  extern ItCoroutine itCoroutine;
  extern std::set<std::wstring> strings;

  struct GlobalDefsComp {
    bool operator ()(const std::wstring *a, const std::wstring *b) const noexcept;
  };

  extern std::map<const std::wstring *, Value, GlobalDefsComp> globalDefs;

  int init();
  int yield();
}
