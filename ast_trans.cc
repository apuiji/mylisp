#include<cmath>
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
  int transSymbol<token::symbol("!")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2<Operation1<1, token::symbol("!")>>(dest, defs, pos, src);
  }

  template<class T>
  static inline int trans2a(UNode &dest, Defs &defs, const Pos *pos, UNode &src, double d = NAN, double d1 = d) {
    UNodes a;
    trans(a, defs, src);
    switch (a.size()) {
      case 0: {
        a.emplace_back(new NumberAtom(nullptr, d));
        [[fallthrough]];
      }
      case 1: {
        a.emplace_back(new NumberAtom(nullptr, d1));
        [[fallthrough]];
      }
    }
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<token::symbol("%")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("%")>>(dest, defs, pos, src);
  }

  template<class T>
  static inline int trans3(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNodes a;
    trans(a, defs, src);
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<token::symbol("&&")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans3<Operation1<-1, token::symbol("&&")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("&")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("&")>>(dest, defs, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol("**")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("**")>>(dest, defs, pos, src, NAN, 1);
  }

  template<>
  int transSymbol<token::symbol("*")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("*")>>(dest, defs, pos, src, NAN, 1);
  }

  template<>
  int transSymbol<token::symbol("+")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("+")>>(dest, defs, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol(",")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans(dest, defs, src);
  }

  template<>
  int transSymbol<token::symbol("-")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNodes items;
    trans(items, defs, src);
    if (items.empty()) {
      dest.reset(new NumberAtom(nullptr, 0));
      return 0;
    }
    if (items.size() == 1) {
      items.push_back(std::move(items[0]));
      items[0].reset(new NumberAtom(nullptr, 0));
    }
    dest.reset(new Operation1<-1, token::symbol("-")>(pos, std::move(items)));
    return 0;
  }

  template<class T>
  static inline int trans4(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNodes a;
    trans(a, defs, src);
    switch (a.size()) {
      case 0: {
        a.emplace_back(new Null);
        [[fallthrough]];
      }
      case 1: {
        a.emplace_back(new Null);
      }
    }
    dest.reset(new T(pos, std::move(a)));
  }

  template<>
  int transSymbol<token::symbol(".")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans4<Operation1<-1, token::symbol(".")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("/")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("/")>>(dest, defs, pos, src, NAN, 1);
  }

  template<>
  int transSymbol<token::symbol("<<")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("<<")>>(dest, defs, pos, src, NAN, 0);
  }

  template<class T>
  static inline int trans5(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    array<UNode, 2> a;
    if (src && src->next) {
      trans(a[1], defs, src->next);
    } else {
      a[1].reset(new Null);
    }
    if (src) {
      trans1(a[0], defs, src);
    } else {
      a[0].reset(new Null);
    }
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<token::symbol("<=")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans5<Operation1<2, token::symbol("<=")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("<")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans5<Operation1<2, token::symbol("<")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("==")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans5<Operation1<2, token::symbol("==")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("=")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    UNode a;
    UNode b;
    if (src && src->next) {
      trans(b, defs, src->next);
    } else {
      b.reset(new Null);
    }
    if (src) {
      trans1(a, defs, src);
    } else {
      throw Trans(pos, "nothing assign");
    }
    if (dynamic_cast<const IDAtom *>(a.get())) {
      dest.reset(new Operation1<2, token::symbol("=")>(pos, { std::move(a), std::move(b) }));
    } else if (auto c = dynamic_cast<Operation1<-1, token::symbol("=")> *>(a.get()); c) {
      if (c->items.size() == 2) {
        dest.reset(new SetMemberOper(pos, { std::move(c->items[0]), std::move(c->items[1]), std::move(b) }));
      } else {
        auto d = std::move(c->items.back());
        c->items.pop_back();
        dest.reset(new SetMemberOper(pos, { std::move(a), std::move(d), std::move(b) }));
      }
    } else {
      throw TransBad(pos, "illegal assign");
    }
    return 0;
  }

  template<>
  int transSymbol<token::symbol(">=")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans5<Operation1<2, token::symbol(">=")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol(">>>")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol(">>>")>>(dest, defs, pos, src, NAN, 0);
  }

  template<>
  int transSymbol<token::symbol(">>")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol(">>")>>(dest, defs, pos, src, NAN, 0);
  }

  template<>
  int transSymbol<token::symbol(">")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans5<Operation1<2, token::symbol(">")>>(dest, defs, pos, src);
  }

  static int fnParams(vector<const wstring *> &dest, Defs &defs, const UNode &src);

  template<>
  int transSymbol<token::symbol("@")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    auto ls = dynamic_cast<const List *>(src.get());
    if (!ls) {
      throw TransBad(pos, "required function parameter list");
    }
    vector<const wstring *> params;
    Defs defs1;
    fnParams(params, defs1, ls->first);
    UNode body;
    if (src->next) {
      trans(body, defs1, src->next);
    } else {
      body.reset(new Null);
    }
    dest.reset(new Function(pos, std::move(defs1), std::move(params), std::move(body)));
    return 0;
  }

  template<>
  int transSymbol<token::symbol("^^")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans3<Operation1<-1, token::symbol("^^")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("^")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("^")>>(dest, defs, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol("||")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans3<Operation1<-1, token::symbol("||")>>(dest, defs, pos, src);
  }

  template<>
  int transSymbol<token::symbol("|")>(UNode &dest, Defs &defs, const Pos *pos, UNode &src) {
    return trans2a<Operation1<-1, token::symbol("|")>>(dest, defs, pos, src, 0);
  }
}
