#pragma once

#include<concepts>
#include<filesystem>
#include<map>
#include<memory>
#include<set>
#include<string>
#include<utility>
#include<vector>

namespace zlt::mylisp::ast {
  using Pos = std::pair<const std::filesystem::path *, int>;

  int pos2str(std::wstring &dest, const Pos &src);

  template<class Ostr, class It>
  requires requires (It it) {
    { **it } -> std::same_as<const Pos &>;
  }
  int poss2str(Ostr &ostr, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    std::wstring s;
    pos2str(s, **it);
    ostr << '\n' << s;
    return poss2str(ostr, ++it, end);
  }

  struct Node;

  using UNode = std::unique_ptr<Node>;
  using UNodes = std::vector<UNode>;

  struct Node {
    const Pos *pos;
    UNode next;
    Node(const Pos *pos = nullptr) noexcept: pos(pos) {}
    virtual ~Node() = default;
  };

  static inline int replace(UNode &dest, UNode &src) noexcept {
    src->next = std::move(dest->next);
    dest = std::move(src);
    return 0;
  }

  static inline int replace(UNode &dest, UNode &&src) noexcept {
    return replace(dest, src);
  }

  UNode shift(UNode &src) noexcept;

  struct Macro {
    using Params = std::vector<const std::wstring *>;
    using ItParam = Params::const_iterator;
    Params params;
    UNode body;
    Macro() = default;
    Macro(Params &&params, UNode &&body) noexcept: params(std::move(params)), body(std::move(body)) {}
  };

  struct Ast {
    using Sources = std::map<const std::filesystem::path *, std::wstring>;
    using Loadeds = std::map<const std::filesystem::path *, UNode>;
    using ItSource = Sources::const_iterator;
    using ItLoaded = Loadeds::const_iterator;
    std::set<std::filesystem::path> files;
    std::map<const std::filesystem::path *, std::wstring> sources;
    Loadeds loadeds;
    std::set<Pos> positions;
    std::map<const std::wstring *, Macro> macros;
    int operator ()(UNode &dest, const std::filesystem::path &src);
  };

  struct AstBad {
    std::wstring what;
    AstBad(std::wstring &&what) noexcept: what(std::move(what)) {}
  };
}
