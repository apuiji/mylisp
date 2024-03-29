#include<algorithm>
#include"ast_nodes.hh"
#include"ast_token.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const char *;

  static It node(UNode &dest, Lexer &lexer, It start0, It end);
  static It nodes(UNodes &dest, Lexer &lexer, It end0, It end);

  int parse(UNode &dest, It it, It end) {
    Lexer lexer;
    It end0 = parse(dest, lexer, it, end);
    It start1 = hit(end0, end);
    auto [_1, end1] = lexer(start1, end);
    if (_1 != token::E0F) {
      throw AstBad(bad::UNEXPECTED_TOKEN, start1);
    }
    return 0;
  }

  It node(UNode &dest, Lexer &lexer, It start0, It end) {
    auto [_0, end0] = lexer(start0, end);
    if (_0 == ")"_token || _0 == token::E0F) {
      return nullptr;
    }
    if (_0 == token::NUMBER) {
      dest.reset(new NumberAtom(start0, lexer.raw, lexer.numval));
      return end0;
    }
    if (_0 == token::CHAR) {
      dest.reset(new CharAtom(start0, lexer.charval));
      return end0;
    }
    if (_0 == token::STRING) {
      auto value = rte::addString(std::move(lexer.strval));
      dest.reset(new StringAtom(start0, value));
      return end0;
    }
    if (_0 == token::ID) {
      auto name = rte::addString(lexer.raw);
      dest.reset(new IDAtom(start0, lexer.raw, name));
      return end0;
    }
    if (_0 == "("_token) {
      UNodes items;
      It end1 = parse(items, lexer, end0, end);
      It start2 = hit(end1, end);
      auto [_2, end2] = lexer(start2, end);
      if (_2 != ")"_token) {
        throw AstBad(bad::UNTERMINATED_LIST, start0);
      }
      dest.reset(new List(start0, std::move(items)));
      return end2;
    }
    dest.reset(new TokenAtom(start0, lexer.raw, _0));
    return end0;
  }

  It nodes(UNodes &dest, Lexer &lexer, It end0, It end) {
    {
      UNode _1;
      It start1 = hit(end0, end);
      It end1 = node(_1, lexer, start1, end);
      if (!end1) {
        return end0;
      }
      dest.push_back(std::move(_1));
      end0 = end1;
    }
    return nodes(dest, lexer, end0, end);
  }
}
