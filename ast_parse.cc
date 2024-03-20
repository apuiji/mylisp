#include<algorithm>
#include"ast_parse.hh"
#include"ast_token.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const char *;

  static It parse(UNodes &dest, Lexer &lexer, It it, It end);

  int parse(UNodes &dest, It it, It end) {
    Lexer lexer;
    It end0 = parse(dest, lexer, it, end);
    auto [_1, start1, end1] = lexer(end0, end);
    if (_1 != token::E0F) {
      throw AstBad(bad::UNEXPECTED_TOKEN, start1);
    }
    return 0;
  }

  static It parse1(UNode &dest, Lexer &lexer, It it, It end);

  It parse(UNodes &dest, Lexer &lexer, It it, It end) {
    It it1;
    {
      UNode a;
      it1 = parse1(a, lexer, it, end);
      if (!it1) {
        return it;
      }
      dest.push_back(std::move(a));
    }
    return parse(dest, lexer, it1, end);
  }

  It parse1(UNode &dest, Lexer &lexer, It it, It end) {
    auto [t0, start0, end0] = lexer(it, end);
    if (t0 == token::E0F || t0 == ")"_token) [[unlikely]] {
      return nullptr;
    }
    if (t0 == token::NUMBER) {
      dest.reset(new NumberAtom(start0, lexer.raw, lexer.numval));
      return end0;
    }
    if (t0 == token::CHAR) {
      dest.reset(new CharAtom(start0, lexer.charval));
      return end0;
    }
    if (t0 == token::STRING) {
      auto value = rte::addString(std::move(lexer.strval));
      dest.reset(new StringAtom(start0, value));
      return end0;
    }
    if (t0 == token::ID) {
      auto name = rte::addString(lexer.raw);
      dest.reset(new IDAtom(start0, name));
      return end0;
    }
    if (to == "("_token) {
      UNodes items;
      It end1 = parse(items, lexer, end0, end);
      auto [t2, start2, end2] = lexer(end1, end);
      if (t2 != ")"_token) {
        throw AstBad(bad::UNTERMINATED_LIST, start0);
      }
      dest.reset(new List(start0, std::move(items)));
      return end2;
    }
    dest.reset(new TokenAtom(start0, lexer.raw, t0));
    return end0;
  }

  int clone(UNode &dest, const UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<const T *>(src.get()); a) { \
      dest.reset(new T(*a)); \
      return 0; \
    }
    ifType(NumberAtom);
    ifType(CharAtom);
    ifType(StringAtom);
    ifType(IDAtom);
    ifType(TokenAtom);
    #undef ifType
    auto ls = static_cast<const List *>(src.get());
    UNodes items;
    clones(items, ls->items.begin(), ls->items.end());
    dest.reset(new List(ls->start, std::move(items)));
    return 0;
  }

  int clone(UNodes &dest, const UNode &src) {
    UNode a;
    clone(a, src);
    dest.push_back(std::move(a));
    return 0;
  }

  int clones(UNodes &dest, UNodes::const_iterator it, UNodes::const_iterator end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    clone(dest, *it);
    return clones(dest, ++it, end);
  }
}
