#pragma once

#include<set>
#include"coroutine.hh"
#include"macro.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern Coroutines coroutines;
  extern std::set<std::filesystem::path> files;
  extern std::set<std::string> fnBodies;
  extern std::map<const std::wstring *, Value> globalDefs;
  extern ItCoroutine itCoroutine;
  extern std::set<std::string> latin1s;
  extern std::map<const std::wstring *, Macro> macros;
  extern std::set<ast::Pos> positions;
  extern std::set<std::wstring> strings;

  int init();
  int yield();
}
