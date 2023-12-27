#include<algorithm>
#include"ast_parse.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const wchar_t *;

  struct Parse {
    // source begin
    const filesystem::path &file;
    It begin;
    It end;
    // source end
    Lexer &lexer;
    It operator ()(UNode &dest, UNode *&next, It it);
  };

  Pos makePos(const filesystem::path &file, It begin, It end, It it) noexcept;

  UNode &parse(UNode &dest, const filesystem::path &file, It begin, It end) {
    Lexer lexer;
    It it;
    UNode *next;
    try {
      it = Parse(file, begin, end, lexer)(dest, next, begin);
    } catch (LexerBad bad) {
      throw ParseBad(makePos(file, begin, end, bad.start), std::move(bad.what));
    }
    auto [t, start1, end1] = lexer(it, end);
    if (t != token::E0F) {
      throw ParseBad(makePos(file, begin, end, start1), "unexpected token");
    }
    return *next;
  }

  static int getPosLi(int li, It line, It end, It it) noexcept;

  Pos makePos(const filesystem::path &file, It begin, It end, It it) noexcept {
    return Pos(file, getPosLi(0, begin, end, it));
  }

  int getPosLi(int li, It line, It end, It it) noexcept {
    It eol = find(line, end, '\n');
    return eol > it ? li : getPosLi(li + 1, eol + 1, end, it);
  }

  const Pos *makePos1(const filesystem::path &file, It begin, It end, It it) noexcept {
    return &*rte::positions.insert(makePos(file, begin, end, it)).first;
  }

  It Parse::operator ()(UNode &dest, UNode *&next, It it) {
    auto [t0, start0, end0] = lexer(it, end);
    switch (t0) {
      case token::RPAREN:
        [[fallthrough]];
      [[unlikely]] case token::E0F: {
        next = &dest;
        return start0;
      }
      case token::NUMBER: {
        dest.reset(new NumberAtom(makePos1(file, begin, end, start0), lexer.numval));
        return operator ()(dest->next, end0);
      }
      case token::CHAR: {
        dest.reset(new CharAtom(makePos1(file, begin, end, start0), lexer.charval));
        return operator ()(dest->next, end0);
      }
      case token::STRING: {
        auto &value = *rte::latin1s.insert(std::move(lexer.strval)).first;
        dest.reset(new Latin1Atom(makePos1(file, begin, end, start0), value));
        return operator ()(dest->next, end0);
      }
      case token::WSTRING: {
        auto &value = *rte::strings.insert(std::move(lexer.wstrval)).first;
        dest.reset(new StringAtom(makePos1(file, begin, end, start0), value));
        return operator ()(dest->next, end0);
      }
      case token::ID: {
        auto &name = *rte::strings.insert(lexer.raw).first;
        dest.reset(new IDAtom(makePos1(file, begin, end, start0), name));
        return operator ()(dest->next, end0);
      }
      case token::LPAREN: {
        UNode first;
        It end1 = operator ()(first, rtol<UNode *>(), end0);
        auto [t2, start2, end2] = lexer(end1, end);
        if (t2 != token::RPAREN) {
          throw ParseBad(makePos(file, begin, end, start0), "unterminated list");
        }
        dest.reset(new List(makePos1(file, begin, end, start0), std::move(first)));
        return operator ()(dest->next, end2);
      }
      default: {
        dest.reset(new TokenAtom(makePos1(file, begin, end, start0), t0));
        return operator ()(dest->next, end0);
      }
    }
  }
}
