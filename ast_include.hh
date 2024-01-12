#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  UNode &include(UNode &dest, const std::filesystem::path &file);

  struct IncludeBad {
    std::filesystem::path file;
    std::wstring what;
    IncludeBad(const std::filesystem::path &file, std::wstring &&what) noexcept: file(file), what(std::move(what)) {}
  };
}
