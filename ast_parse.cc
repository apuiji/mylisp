#include<algorithm>
#include"ast_lexer.hh"
#include"ast_parse.hh"
#include"ast_token.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const char *;

  static It parse(UNode &dest, Lexer &lexer, It it, It end);

  int parse(UNode &dest, It it, It end) {
    Lexer lexer;
    It end0 = parse(dest, lexer, it, end);
    auto [_1, start1, end1] = lexer(end0, end);
    if (_1 == token::E0F) {
      throw AstBad(bad::UNEXPECTED_TOKEN, start1);
    }
    return 0;
  }

  It parse(UNode &dest, Lexer &lexer, It it, It end) {
    auto [t0, start0, end0] = lexer(it, end);
    switch (t0) {
      case ")"_token:
        [[fallthrough]];
      [[unlikely]] case token::E0F: {
        return start0;
      }
      case token::NUMBER: {
        dest.reset(new NumberAtom(start0, lexer.raw, lexer.numval));
        return parse(dest->next, lexer, end0, end);
      }
      case token::CHAR: {
        dest.reset(new CharAtom(start0, lexer.charval));
        return parse(dest->next, lexer, end0, end);
      }
      case token::STRING: {
        auto value = rte::addString(std::move(lexer.strval));
        dest.reset(new StringAtom(start0, value));
        return parse(dest->next, lexer, end0, end);
      }
      case token::ID: {
        auto name = rte::addString(lexer.raw);
        dest.reset(new IDAtom(start0, name));
        return parse(dest->next, lexer, end0, end);
      }
      case "("_token: {
        UNode first;
        It end1 = parse(first, lexer, end0, end);
        auto [t2, start2, end2] = lexer(end1, end);
        if (t2 != ")"_token) {
          throw AstBad(bad::UNTERMINATED_LIST, start0);
        }
        dest.reset(new List(start0, std::move(first)));
        return parse(dest->next, lexer, end2, end);
      }
      default: {
        dest.reset(new TokenAtom(start0, lexer.raw, t0));
        return parse(dest->next, lexer, end0, end);
      }
    }
  }

  int clone(UNode &dest, const UNode &src) {
    if (auto a = dynamic_cast<const NumberAtom *>(src.get()); a) {
      dest.reset(new NumberAtom(a->start, a->raw, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const CharAtom *>(src.get()); a) {
      dest.reset(new CharAtom(a->start, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.get()); a) {
      dest.reset(new StringAtom(a->start, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest.reset(new IDAtom(a->start, a->name));
      return 0;
    }
    if (auto a = dynamic_cast<const TokenAtom *>(src.get()); a) {
      dest.reset(new TokenAtom(a->start, a->raw, a->token));
      return 0;
    }
    auto ls = static_cast<const List *>(src.get());
    UNode first;
    clones(first, ls->first);
    dest.reset(new List(ls->start, std::move(first)));
    return 0;
  }

  UNode &clones(UNode &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
    clone(dest, src);
    return clones(dest->next, src->next);
  }
}
