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
    Node(const char *start = nullptr) noexcept: start(start) {}
    virtual ~Node() = default;
  };

  struct Macro {
    using Params = std::vector<const std::string *>;
    using ItParam = Params::const_iterator;
    Params params;
    UNode body;
    Macro() = default;
    Macro(Params &&params, UNode &&body) noexcept: params(std::move(params)), body(std::move(body)) {}
  };

  using Source = std::pair<std::string, UNode>;
  using Sources = std::map<std::filesystem::path, Source>;
  using ItSource = Sources::const_iterator;

  struct Ast {
    Sources sources;
    std::map<const std::string *, Macro> macros;
    int operator ()(UNode &dest, const std::filesystem::path &src);
  };

  ItSource whichSource(const Ast &ast, const char *start) noexcept;

  struct AstBad {
    int code;
    const char *start;
    AstBad(int code, const char *start) noexcept: code(code), start(start) {}
  };

  namespace bad {
    enum {
      ASSIGN_NOTHING,
      CANNOT_OPEN_SRC_FILE,
      DEF_NOTHING,
      ILLEGAL_FN_PARAM,
      ILLEGAL_LHS,
      ILLEGAL_MACRO_PARAM,
      ILLEGAL_PREPROC_ARG,
      MACRO_ALREADY_EXISTS,
      MACRO_REST_PARAM_MUST_BE_LAST,
      NUMBER_LITERAL_OOR,
      SHOULD_NOT_IN_GLOBAL,
      UNEXPECTED_TOKEN,
      UNRECOGNIZED_SYMBOL,
      UNTERMINATED_LIST,
      UNTERMINATED_STRING
    };
  }
}
