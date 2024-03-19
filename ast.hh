#pragma once

#include<concepts>
#include<filesystem>
#include<map>
#include<memory>
#include<set>
#include<utility>
#include<vector>
#include"myccutils/myiter.hh"

namespace zlt::mylisp::ast {
  struct Node;

  using UNode = std::unique_ptr<Node>;
  using UNodes = std::vector<UNode>;

  struct Node {
    const char *start;
    UNode next;
    Node(const char *start = nullptr) noexcept: start(start) {}
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
    std::map<const std::string *, Macro> macros;
    int operator ()(UNode &dest, const std::filesystem::path &src);
  };

  struct AstBad {
    int code;
    const char *start;
    AstBad(int code, const char *start) noexcept: code(code), start(start) {}
  };

  namespace bad {
    enum {
      CANNOT_OPEN_SRC_FILE,
      ILLEGAL_PREPROC_ARG,
      MACRO_ALREADY_EXISTS,
      NUMBER_LITERAL_OOR,
      UNEXPECTED_TOKEN,
      UNRECOGNIZED_SYMBOL,
      UNTERMINATED_LIST,
      UNTERMINATED_STRING
    };
  }
}
