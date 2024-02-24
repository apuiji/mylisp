#pragma once

#include<concepts>
#include<filesystem>
#include<map>
#include<memory>
#include<ostream>
#include<set>
#include<utility>
#include<vector>

namespace zlt::mylisp::ast {
  using Pos = std::pair<const std::filesystem::path *, int>;

  std::ostream &operator <<(std::ostream &dest, const Pos &pos);

  template<RangeOf<Pos> T>
  std::ostream &operator <<(std::ostream &dest, const T &t) {
    for (const Pos &pos : r) {
      dest << pos << std::endl;
    }
    return dest;
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
    using Params = std::vector<const std::string *>;
    using ItParam = Params::const_iterator;
    Params params;
    UNode body;
    Macro() = default;
    Macro(Params &&params, UNode &&body) noexcept: params(std::move(params)), body(std::move(body)) {}
  };

  struct Ast {
    using Sources = std::map<const std::filesystem::path *, std::string>;
    using Loadeds = std::map<const std::filesystem::path *, UNode>;
    using ItSource = Sources::const_iterator;
    using ItLoaded = Loadeds::const_iterator;
    std::set<std::filesystem::path> files;
    std::map<const std::filesystem::path *, std::string> sources;
    Loadeds loadeds;
    std::set<Pos> positions;
    std::map<const std::string *, Macro> macros;
    int operator ()(UNode &dest, const std::filesystem::path &src);
  };

  struct AstBad {
    std::string what;
    AstBad(std::string &&what) noexcept: what(std::move(what)) {}
  };
}
