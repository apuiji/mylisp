#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  UNode &include(UNode &dest, const std::filesystem::path &file);
}
