#pragma once

#include<map>
#include<set>
#include"macro.hh"

/// runtime environment
namespace zlt::mylisp::rte {
  extern std::set<std::filesystem::path> files;
  extern std::set<std::string> latin1s;
  extern std::map<const std::wstring *, Macro> macros;
  extern std::set<ast::Pos> positions;
  extern std::set<std::wstring> strings;
}
