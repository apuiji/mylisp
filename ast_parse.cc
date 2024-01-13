#include<algorithm>
#include"ast_lexer.hh"
#include"ast_parse.hh"
#include"ast_token.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const wchar_t *;

  struct Parse {
    // source begin
    const filesystem::path *file;
    const wchar_t *start;
    const wchar_t *end;
    // source end
    Ast &ast;
    Lexer &lexer;
    Parse(const filesystem::path *file, It start, It end, Ast &ast, Lexer &lexer) noexcept:
    file(file), start(start), end(end), ast(ast), lexer(lexer) {}
    It operator ()(UNode &dest, It it);
  };

  static int getPosLi(int li, It line, It end, It it) noexcept;

  static inline Pos makePos(const filesystem::path *file, It start, It end, It it) noexcept {
    return Pos(file, getPosLi(0, start, end, it));
  }

  int parse(UNode &dest, Ast &ast, const filesystem::path *file, It start, It end) {
    Lexer lexer;
    It it;
    try {
      it = Parse(file, start, end, ast, lexer)(dest, start);
    } catch (LexerBad bad) {
      throw ParseBad(makePos(file, start, end, bad.start), std::move(bad.what));
    }
    auto [t, start1, end1] = lexer(it, end);
    if (t != token::E0F) {
      throw ParseBad(makePos(file, start, end, start1), L"unexpected token");
    }
    return 0;
  }

  int getPosLi(int li, It line, It end, It it) noexcept {
    It eol = find(line, end, '\n');
    return eol > it ? li : getPosLi(li + 1, eol + 1, end, it);
  }

  const Pos *makePos1(Ast &ast, const filesystem::path *file, It start, It end, It it) noexcept {
    return &*ast.positions.insert(makePos(file, start, end, it)).first;
  }

  It Parse::operator ()(UNode &dest, It it) {
    auto [t0, start0, end0] = lexer(it, end);
    switch (t0) {
      case token::RPAREN:
        [[fallthrough]];
      [[unlikely]] case token::E0F: {
        return start0;
      }
      case token::NUMBER: {
        dest.reset(new NumberAtom(makePos1(ast, file, start, end, start0), lexer.raw, lexer.numval));
        return operator ()(dest->next, end0);
      }
      case token::CHAR: {
        dest.reset(new CharAtom(makePos1(ast, file, start, end, start0), lexer.charval));
        return operator ()(dest->next, end0);
      }
      case token::STRING: {
        auto &value = *rte::strings.insert(std::move(lexer.strval)).first;
        dest.reset(new StringAtom(makePos1(ast, file, start, end, start0), &value));
        return operator ()(dest->next, end0);
      }
      case token::ID: {
        auto &name = *rte::strings.insert((wstring) lexer.raw).first;
        dest.reset(new IDAtom(makePos1(ast, file, start, end, start0), &name));
        return operator ()(dest->next, end0);
      }
      case token::LPAREN: {
        UNode first;
        It end1 = operator ()(first, end0);
        auto [t2, start2, end2] = lexer(end1, end);
        if (t2 != token::RPAREN) {
          throw ParseBad(makePos(file, start, end, start0), L"unterminated list");
        }
        dest.reset(new List(makePos1(ast, file, start, end, start0), std::move(first)));
        return operator ()(dest->next, end2);
      }
      default: {
        dest.reset(new TokenAtom(makePos1(ast, file, start, end, start0), lexer.raw, t0));
        return operator ()(dest->next, end0);
      }
    }
  }

  int clone(UNode &dest, const UNode &src) {
    if (auto a = dynamic_cast<const NumberAtom *>(src.get()); a) {
      dest.reset(new NumberAtom(a->pos, a->raw, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const CharAtom *>(src.get()); a) {
      dest.reset(new CharAtom(a->pos, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.get()); a) {
      dest.reset(new StringAtom(a->pos, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest.reset(new IDAtom(a->pos, a->name));
      return 0;
    }
    if (auto a = dynamic_cast<const TokenAtom *>(src.get()); a) {
      dest.reset(new TokenAtom(a->pos, a->raw, a->token));
      return 0;
    }
    auto ls = static_cast<const List *>(src.get());
    UNode first;
    clones(first, ls->first);
    dest.reset(new List(ls->pos, std::move(first)));
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
