#pragma once

#include<filesystem>
#include<map>
#include"ast_parse.hh"

namespace zlt::mylisp::ast {
  UNode &preproc(UNode &dest, std::map<const std::filesystem::path *, UNode> &loadeds, const UNode &src);

  struct PreprocBad {
    std::wstring what;
    std::vector<const Pos *> posk;
    PreprocBad(std::wstring &&what) noexcept: what(std::move(what)) {}
    PreprocBad(std::wstring &&what, std::vector<const Pos *> &&posk) noexcept: what(std::move(what)), posk(std::move(posk)) {}
    PreprocBad(std::wstring &&what, const Pos *pos): what(std::move(what)) {
      posk.push_back(pos);
    }
    PreprocBad(PreprocBad &&bad, const Pos *pos): PreprocBad(std::move(bad)) {
      posk.push_back(pos);
    }
  };
}
