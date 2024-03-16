#include<algorithm>
#include"ast_parse.hh"
#include"ast_token.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const char *;

  static It nodes(UNodes &_0, It _0end, It end, Lexer &lexer);
  static It nodes1(UNodes &_0, It _0end, It end, Lexer &lexer);
  static It node(UNode &__, It _0start, It end, Lexer &lexer);

  static inline int expect(bool b, It start) {
    if (!b) {
      throw AstBad(bad::UNEXPECTED_TOKEN, start);
    }
    return 0;
  }

  int parse(UNodes &dest, It start, It end) {
    Lexer lexer;
    It _0end = nodes(dest, start, end, lexer);
    It _1start = hit(_0end, end);
    auto [_1, _1end] = lexer(_1start, end);
    expect(_1 == token::E0F, _1start);
    return 0;
  }

  It nodes(UNodes &_0, It _0end, It end, Lexer &lexer) {
    It _1end = nodes1(_0, _0end, end, lexer);
    return _1end ? nodes(_0, _1end, end, lexer) : _0end;
  }

  It nodes1(UNodes &_0, It _0end, It end, Lexer &lexer) {
    UNode _1;
    It _1start = hit(_0end, end);
    It _1end = node(_1, _1start, end, lexer);
    if (!_1end) {
      return nullptr;
    }
    _0.push_back(std::move(_1));
    return _1end;
  }

  It node(UNode &__, It _0start, It end, Lexer &lexer) {
    auto [_0, _0end] = lexer(_0start, end);
    if (_0 == token::E0F || _0 == ")"_token) [[unlikely]] {
      return nullptr;
    }
    if (_0 == token::NUMBER) {
      dest.reset(new NumberAtom(_0start, lexer.raw, lexer.numval));
      return _0end;
    }
    if (_0 == token::CHAR) {
      dest.reset(new CharAtom(_0start, lexer.charval));
      return _0end;
    }
    if (_0 == token::STRING) {
      auto &value = *rte::strings.insert(std::move(lexer.strval)).first;
      dest.reset(new StringAtom(_0start, &value));
      return _0end;
    }
    if (_0 == token::ID) {
      dest.reset(new IDAtom(_0start, lexer.raw));
      return _0end;
    }
    if (_0 == "("_token) {
      UNodes _1;
      It _1end = nodes(_1, _0end, end, lexer);
      It _2start = hit(_1end, end);
      auto [_2, _2end] = lexer(_2start, end);
      expect(_2 == ")"_token, _2start);
      dest.reset(new List(_0start, std::move(_1)));
      return _2end;
    }
    dest.reset(new TokenAtom(_0start, lexer.raw, _0));
    return _0end;
  }
}
