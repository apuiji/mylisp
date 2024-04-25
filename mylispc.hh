#pragma once

#include<filesystem>
#include<list>
#include<map>
#include<memory>
#include<set>
#include<string>
#include<vector>

namespace zlt::mylispc {
  struct Node {
    const char *start;
    Node(const char *start = nullptr) noexcept: start(start) {}
    virtual ~Node() = default;
  };

  using UNode = std::unique_ptr<Node>;
  using UNodes = std::list<UNode>;

  struct Source {
    std::string raw;
    UNodes body;
    Source(std::string &&raw, UNodes &&body) noexcept: raw(std::move(raw)), body(std::move(body)) {}
  };

  using Sources = std::map<std::filesystem::path, Source>;

  Sources::const_iterator where(Sources &srcs, const char *start) noexcept;

  struct Macro {
    using Params = std::vector<const std::string *>;
    using ItParam = Params::const_iterator;
    Params params;
    UNodes body;
    Macro() = default;
    Macro(Params &&params, UNodes &&body) noexcept: params(std::move(params)), body(std::move(body)) {}
  };

  using Macros = std::map<const std::string *, Macro>;

  const char *hit(const char *it, const char *end) noexcept;
  /// @param[out] numval when token is NUMBER
  /// @param[out] charval when token is CHAR
  /// @param[out] strval when token is STRING
  /// @return [token, end]
  std::pair<int, const char *> lexer(double &numval, char &charval, std::string &strval, const char *it, const char *end);

  static inline std::pair<int, const char *> lexer(const char *it, const char *end) {
    double d;
    char c;
    std::string s;
    return lexer(d, c, s, it, end);
  }

  void parse(UNodes &dest, const char *it, const char *end);
  void preproc(UNodes &dest, Sources &srcs, Macros &macros, const UNode &src);

  static inline void preproc(
    UNodes &dest, Sources &srcs, Macros &macros, UNodes::const_iterator it, UNodes::const_iterator end) {
    for (; it != end; ++it) {
      preproc(dest, srcs, macros, *it);
    }
  }

  void trans(UNodes &src);
  void optimize(UNode &src);

  template<class It>
  requires (std::is_same_v<std::iter_value_t<It>, UNode>)
  static inline void optimize(It it, It end) {
    for (; it != end; ++it) {
      optimize(*it);
    }
  }

  /// @return terminated
  bool optimizeBody(UNodes &dest, UNodes::iterator it, UNodes::iterator end);
  /// @param[out] dest main function
  void trans1(UNode &dest, UNodes::iterator it, UNodes::iterator end);
  void trans2(UNode &src);
  void compile(std::string &dest, const UNode &src);

  struct Bad {
    int code;
    const char *start;
    Bad(int code, const char *start) noexcept: code(code), start(start) {}
  };

  namespace bad {
    enum {
      CANNOT_OPEN_FILE,
      ILL_FN_PARAM,
      ILL_MACRO_PARAM,
      INV_PREPROC_ARG,
      MACRO_ALREADY_DEFINED,
      NUMBER_LITERAL_OOR,
      UNEXPECTED_TOKEN,
      UNRECOGNIZED_SYMBOL,
      UNTERMINATED_LIST,
      UNTERMINATED_STRING
    };
  }
}
