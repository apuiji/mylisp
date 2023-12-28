#include"ast_token.hh"
#include"ast_trans.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using Defs = set<const wstring *>;

  static int trans(UNode &dest, Defs &defs, UNode &src);

  int trans(UNode &dest, UNode &src) {
    return trans(dest, rtol(Defs()), src);
  }

  static int trans1(UNode &dest, Defs &defs, UNode &src);

  int trans(UNode &dest, Defs &defs, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    {
      auto a = std::move(src);
      src = std::move(a->next);
      trans1(dest, defs, a);
    }
    return trans(dest->next, defs, src);
  }

  using Trans = int (UNode &dest, Defs &defs, const Pos *pos, UNode &src);

  static Trans *isTrans(const UNode &src) noexcept;

  static int trans(UNodes &dest, Defs &defs, UNode &src);

  int trans1(UNode &dest, Defs &defs, UNode &src) {
    if (auto t = dynamic_cast<const TokenAtom *>(src.get()); t) {
      if (t->token == token::KWD_callee) {
        dest.reset(new Callee(src->pos));
        return 0;
      }
      throw TransBad(src->pos, "unexpected token");
    }
    if (auto ls = dynamic_cast<List *>(src.get()); ls) {
      if (!ls->first) {
        dest.reset(new Null(src->pos));
        return 0;
      }
      if (auto t = isTrans(ls->first); t) {
        return t(dest, defs, ls->pos, ls->first->next);
      }
      {
        UNodes args;
        trans(args, defs, ls->first->next);
        args.shrink_to_fit();
        UNode callee;
        trans1(callee, defs, ls->first);
        dest.reset(new Call(src->pos, std::move(callee), std::move(args)));
      }
      return 0;
    }
    dest = std::move(src);
    return 0;
  }

  static Trans transKWD_def;
  static Trans transKWD_defer;
  static Trans transKWD_forward;
  static Trans transKWD_if;
  static Trans transKWD_length;
  static Trans transKWD_return;
  static Trans transKWD_throw;
  static Trans transKWD_try;
  static Trans transKWD_yield;

  template<uint64_t>
  static Trans transSymbol;

  #define declTransSymbol(symb) \
  template<> \
  Trans transSymbol<token::symbol(symb)>

  declTransSymbol("!");
  declTransSymbol("%");
  declTransSymbol("&&");
  declTransSymbol("&");
  declTransSymbol("**");
  declTransSymbol("*");
  declTransSymbol("+");
  declTransSymbol(",");
  declTransSymbol("-");
  declTransSymbol(".");
  declTransSymbol("/");
  declTransSymbol("<<");
  declTransSymbol("<=>");
  declTransSymbol("<=");
  declTransSymbol("<");
  declTransSymbol("==");
  declTransSymbol("=");
  declTransSymbol(">=");
  declTransSymbol(">>>");
  declTransSymbol(">>");
  declTransSymbol(">");
  declTransSymbol("@");
  declTransSymbol("^^");
  declTransSymbol("^");
  declTransSymbol("||");
  declTransSymbol("|");
  declTransSymbol("~");

  #undef declTransSymbol

  Trans *isTrans(const UNode &src) noexcept {
    auto a = dynamic_cast<const TokenAtom *>(src.get());
    if (!a) {
      return nullptr;
    }
    switch (a->token) {
      #define ifKeyword(kwd) \
      case token::KWD_##kwd: { \
        return transKWD_##kwd; \
      }
      ifKeyword(def);
      ifKeyword(defer);
      ifKeyword(forward);
      ifKeyword(if);
      ifKeyword(length);
      ifKeyword(return);
      ifKeyword(throw);
      ifKeyword(try);
      ifKeyword(yield);
      #undef ifKeyword
      #define ifSymbol(symb) \
      case token::symbol(symb): { \
        return transSymbol<token::symbol(symb)>; \
      }
      ifSymbol("!");
      ifSymbol("%");
      ifSymbol("&&");
      ifSymbol("&");
      ifSymbol("**");
      ifSymbol("*");
      ifSymbol("+");
      ifSymbol(",");
      ifSymbol("-");
      ifSymbol(".");
      ifSymbol("/");
      ifSymbol("<<");
      ifSymbol("<=>");
      ifSymbol("<=");
      ifSymbol("<");
      ifSymbol("==");
      ifSymbol("=");
      ifSymbol(">=");
      ifSymbol(">>>");
      ifSymbol(">>");
      ifSymbol(">");
      ifSymbol("@");
      ifSymbol("^^");
      ifSymbol("^");
      ifSymbol("||");
      ifSymbol("|");
      ifSymbol("~");
      #undef ifSymbol
      default: {
        return nullptr;
      }
    }
  }

  int trans(UNodes &dest, Defs &defs, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    {
      auto a = std::move(src);
      src = std::move(a->next);
      dest.push_back({});
      trans1(dest.back(), defs, a);
    }
    return trans(dest, defs, src);
  }

  int transKWD_def(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw TransBad(pos, "required definition name");
    }
    defs.insert(id->name);
    remove(src->next);
    dest = std::move(src);
    return 0;
  }

  template<class T>
  static inline int trans2(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNode a;
    if (src) {
      trans(a, defs, src);
    } else {
      a.reset(new Null);
    }
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  int transKWD_defer(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Defer>(dest, defs, pos, src);
  }

  int transKWD_forward(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNode callee;
    UNodes args;
    if (src) {
      trans(args, defs, src->next);
      args.shrink_to_fit();
      trans1(callee, defs, src);
    } else {
      callee.reset(new Null);
    }
    dest.reset(new Forward(pos, std::move(callee), std::move(args)));
    return 0;
  }

  int transKWD_if(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNode cond;
    UNode then;
    UNode elze;
    if (src && src->next && src->next->next) {
      trans(elze, defs, src->next->next);
    } else {
      elze.reset(new Null);
    }
    if (src && src->next) {
      trans1(then, defs, src->next);
    } else {
      then.reset(new Null);
    }
    if (src) {
      trans1(cond, defs, src);
    } else {
      cond.reset(new Null);
    }
    dest.reset(new If(pos, std::move(cond), std::move(then), std::move(elze)));
    return 0;
  }

  int transKWD_length(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Operation1<1, token::KWD_length>>(dest, defs, pos, src);
  }

  int transKWD_return(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Return>(dest, defs, pos, src);
  }

  int transKWD_throw(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Throw>(dest, defs, pos, src);
  }

  int transKWD_try(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Try>(dest, defs, pos, src);
  }

  int transKWD_yield(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Yield>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("!")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src);
}
