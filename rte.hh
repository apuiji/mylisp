#pragma once

#include<set>
#include"coroutine.hh"
#include"macro.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern std::list<Coroutine> coroutines;
  extern std::set<std::filesystem::path> files;
  extern std::set<std::string> fnBodies;
  extern std::list<Coroutine>::iterator itCoroutine;
  extern std::set<std::string> latin1s;
  extern std::map<const std::wstring *, Macro> macros;
  extern std::set<ast::Pos> positions;
  extern std::set<std::wstring> strings;
}
