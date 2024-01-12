#pragma once

#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  const std::filesystem::path *load(UNode &dest, std::filesystem::path &&file);

  struct LoadBad {
    std::filesystem::path file;
    std::wstring what;
    LoadBad(const std::filesystem::path &file, std::wstring &&what) noexcept: file(file), what(std::move(what)) {}
  };
}
