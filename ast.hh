#pragma once

#include<concepts>
#include<filesystem>
#include<list>
#include<map>
#include<memory>
#include<set>
#include<utility>
#include<vector>

namespace zlt::mylisp::ast {
  struct Node {
    const char *start;
    Node(const char *start = nullptr) noexcept: start(start) {}
    virtual ~Node() = default;
  };

  using UNode = std::unique_ptr<Node>;
  using UNodes = std::list<UNode>;

  struct Macro {
    using Params = std::vector<const std::string *>;
    using ItParam = Params::const_iterator;
    Params params;
    UNodes body;
    Macro() = default;
    Macro(Params &&params, UNodes &&body) noexcept: params(std::move(params)), body(std::move(body)) {}
  };

  using Sources = std::map<const std::filesystem::path *, std::string>;
  using Loadeds = std::map<const std::filesystem::path *, UNodes>;
  using ItSource = Sources::const_iterator;
  using ItLoaded = Loadeds::const_iterator;

  struct Ast {
    std::set<std::filesystem::path> files;
    std::map<const std::filesystem::path *, std::string> sources;
    Loadeds loadeds;
    std::map<const std::string *, Macro> macros;
    int operator ()(UNodes &dest, const std::filesystem::path &src);
  };

  const std::filesystem::path *whichFile(const Ast &ast, const char *start) noexcept;

  struct AstBad {
    int code;
    const char *start;
    AstBad(int code, const char *start) noexcept: code(code), start(start) {}
  };

  namespace bad {
    enum {
      ILLEGAL_MACRO_PARAM,
      MACRO_ALREADY_DEFINED,
      NUMBER_LITERAL_OOR,
      REQUIRED_PREPROC_ARGS,
      REST_MACRO_PARAM_MUST_BE_LAST,
      SRC_FILE_OPEN_FAILED,
      UNEXPECTED_TOKEN,
      UNRECOGNIZED_SYMBOL,
      UNTERMINATED_STRING
    };
  }
}
