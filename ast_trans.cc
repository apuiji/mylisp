#include<cmath>
#include"ast_token.hh"
#include"ast_trans.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  struct Scope {
    virtual int def(const wstring *name) = 0;
  };

  struct GlobalScope final: Scope {
    int def(const wstring *name) {
      return 0;
    }
  };

  struct FunctionScope final: Scope {
    set<const wstring *> defs;
    int def(const wstring *name) {
      defs.insert(name);
      return 0;
    }
  };

  static int trans(UNode &dest, Scope &scope, UNode &src);

  int trans(UNode &dest, UNode &src) {
    return trans(dest, rtol(GlobalScope()), src);
  }

  static int trans1(UNode &dest, Scope &scope, UNode &src);

  int trans(UNode &dest, Scope &scope, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    {
      auto a = shift(src);
      trans1(dest, scope, a);
    }
    return trans(dest->next, scope, src);
  }

  using Trans = int (UNode &dest, Scope &scope, const Pos *pos, UNode &src);

  static Trans *isTrans(const UNode &src) noexcept;

  static int trans(UNodes &dest, Scope &scope, UNode &src);

  int trans1(UNode &dest, Scope &scope, UNode &src) {
    if (auto t = dynamic_cast<const TokenAtom *>(src.get()); t) {
      switch (t->token) {
        case token::KWD_callee: {
          if (Dynamicastable<FunctionScope> {}(scope)) {
            dest.reset(new Callee(src->pos));
          } else {
            dest.reset(new Null(src->pos));
          }
          return 0;
        }
        default: {
          throw TransBad(src->pos, "unexpected token");
        }
      }
    }
    if (auto ls = dynamic_cast<List *>(src.get()); ls) {
      if (!ls->first) {
        dest.reset(new Null(src->pos));
        return 0;
      }
      if (auto t = isTrans(ls->first); t) {
        return t(dest, scope, ls->pos, ls->first->next);
      }
      {
        UNodes args;
        trans(args, scope, ls->first->next);
        UNode callee;
        trans1(callee, scope, ls->first);
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

  int trans(UNodes &dest, Scope &scope, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    {
      auto a = shift(src);
      dest.push_back({});
      trans1(dest.back(), scope, a);
    }
    return trans(dest, scope, src);
  }

  int transKWD_def(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw TransBad(pos, "required definition name");
    }
    scope.def(id->name);
    remove(src->next);
    dest = std::move(src);
    return 0;
  }

  template<class T>
  static inline int trans2(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNode a;
    if (src) {
      trans(a, scope, src);
    } else {
      a.reset(new Null);
    }
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  static inline int noGlobal(const Scope &scope, const Pos *pos) {
    if (Dynamicastable<GlobalScope> {}(scope)) {
      throw TransBad(pos, "should not in global scope");
    }
    return 0;
  }

  int transKWD_defer(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    noGlobal(scope, pos);
    return trans2<Defer>(dest, scope, pos, src);
  }

  int transKWD_forward(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    noGlobal(scope, pos);
    UNode callee;
    UNodes args;
    if (src) {
      trans(args, scope, src->next);
      trans1(callee, scope, src);
    } else {
      callee.reset(new Null);
    }
    dest.reset(new Forward(pos, std::move(callee), std::move(args)));
    return 0;
  }

  int transKWD_if(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNode cond;
    UNode then;
    UNode elze;
    if (src && src->next && src->next->next) {
      trans(elze, scope, src->next->next);
    } else {
      elze.reset(new Null);
    }
    if (src && src->next) {
      trans1(then, scope, src->next);
    } else {
      then.reset(new Null);
    }
    if (src) {
      trans1(cond, scope, src);
    } else {
      cond.reset(new Null);
    }
    dest.reset(new If(pos, std::move(cond), std::move(then), std::move(elze)));
    return 0;
  }

  int transKWD_length(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2<Operation1<1, token::KWD_length>>(dest, scope, pos, src);
  }

  int transKWD_return(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    noGlobal(scope, pos);
    return trans2<Return>(dest, scope, pos, src);
  }

  int transKWD_throw(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2<Throw>(dest, scope, pos, src);
  }

  static UNode &endOf(UNode &src) noexcept {
    return src ? endOf(src->next) : src;
  }

  int transKWD_try(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNode body;
    trans(body, scope, src);
    endOf(body).reset(new Throw(nullptr, UNode(new Null)));
    dest.reset(new Try(pos, std::move(body)));
    return 0;
  }

  int transKWD_yield(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2<Yield>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("!")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2<LogicNotOper>(dest, scope, pos, src);
  }

  template<class T>
  static inline int trans2a(UNode &dest, Scope &scope, const Pos *pos, UNode &src, double d, double d1) {
    UNodes a;
    trans(a, scope, src);
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

  template<class T>
  static inline int trans2a(UNode &dest, Scope &scope, const Pos *pos, UNode &src, double d = NAN) {
    return trans2a<T>(dest, scope, pos, src, d, d);
  }

  template<>
  int transSymbol<token::symbol("%")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<ArithModOper>(dest, scope, pos, src);
  }

  template<class T>
  static inline int trans3(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNodes a;
    trans(a, scope, src);
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<token::symbol("&&")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans3<LogicAndOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("&")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<BitwsAndOper>(dest, scope, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol("**")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<ArithPowOper>(dest, scope, pos, src, NAN, 1);
  }

  template<>
  int transSymbol<token::symbol("*")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<ArithMulOper>(dest, scope, pos, src, NAN, 1);
  }

  template<>
  int transSymbol<token::symbol("+")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<ArithAddOper>(dest, scope, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol(",")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans(dest, scope, src);
  }

  template<>
  int transSymbol<token::symbol("-")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNodes items;
    trans(items, scope, src);
    if (items.empty()) {
      dest.reset(new NumberAtom(nullptr, 0));
      return 0;
    }
    if (items.size() == 1) {
      items.push_back(std::move(items[0]));
      items[0].reset(new NumberAtom(nullptr, 0));
    }
    dest.reset(new ArithSubOper(pos, std::move(items)));
    return 0;
  }

  template<class T>
  static inline int trans4(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNodes a;
    trans(a, scope, src);
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
    return 0;
  }

  template<>
  int transSymbol<token::symbol(".")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans4<GetMemberOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("/")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<ArithDivOper>(dest, scope, pos, src, NAN, 1);
  }

  template<>
  int transSymbol<token::symbol("<<")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<LshOper>(dest, scope, pos, src, NAN, 0);
  }

  template<class T>
  static inline int trans5(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    array<UNode, 2> a;
    if (src && src->next) {
      trans(a[1], scope, src->next);
    } else {
      a[1].reset(new Null);
    }
    if (src) {
      trans1(a[0], scope, src);
    } else {
      a[0].reset(new Null);
    }
    dest.reset(new T(pos, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<token::symbol("<=>")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans5<CompareOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("<=")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans5<CmpLteqOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("<")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans5<CmpLtOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("==")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans5<CmpEqOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("=")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    UNode a;
    UNode b;
    if (src && src->next) {
      trans(b, scope, src->next);
    } else {
      b.reset(new Null);
    }
    if (src) {
      trans1(a, scope, src);
    } else {
      throw TransBad(pos, "nothing assign");
    }
    if (dynamic_cast<const IDAtom *>(a.get())) {
      dest.reset(new AssignOper(pos, { std::move(a), std::move(b) }));
    } else if (auto c = dynamic_cast<GetMemberOper *>(a.get()); c) {
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
  int transSymbol<token::symbol(">=")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans5<CmpGteqOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol(">>>")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<UshOper>(dest, scope, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol(">>")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<RshOper>(dest, scope, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol(">")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans5<CmpGtOper>(dest, scope, pos, src);
  }

  static int fnParams(vector<const wstring *> &dest, Scope &scope, const UNode &src);

  template<>
  int transSymbol<token::symbol("@")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    auto ls = dynamic_cast<const List *>(src.get());
    if (!ls) {
      throw TransBad(pos, "required function parameter list");
    }
    FunctionScope fnScope;
    vector<const wstring *> params;
    fnParams(params, fnScope, src);
    UNode body;
    trans(body, fnScope, src->next);
    endOf(body).reset(new Return(nullptr, UNode(new Null)));
    dest.reset(new Function(pos, std::move(fnScope.defs), std::move(params), std::move(body)));
    return 0;
  }

  int fnParams(vector<const wstring *> &dest, Scope &scope, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(src.get()); id) {
      dest.push_back(id->name);
      scope.def(id->name);
      return fnParams(dest, scope, src->next);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls && !ls->first) {
      dest.push_back(nullptr);
      return fnParams(dest, scope, src->next);
    }
    throw TransBad(src->pos, "illegal function parameter");
  }

  template<>
  int transSymbol<token::symbol("^^")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans4<LogicXorOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("^")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<BitwsXorOper>(dest, scope, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol("||")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans3<LogicOrOper>(dest, scope, pos, src);
  }

  template<>
  int transSymbol<token::symbol("|")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2a<BitwsOrOper>(dest, scope, pos, src, 0);
  }

  template<>
  int transSymbol<token::symbol("~")>(UNode &dest, Scope &scope, const Pos *pos, UNode &src) {
    return trans2<BitwsNotOper>(dest, scope, pos, src);
  }
}
