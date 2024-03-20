#include<cmath>
#include"ast_token.hh"
#include"ast_trans.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  struct Scope {
    enum {
      GLOBAL_SCOPE_CLASS,
      FUNCTION_SCOPE_CLASS
    };
    int clazz;
    Scope(int clazz) noexcept: clazz(clazz) {}
  };

  struct FunctionScope: Scope {
    Function::Defs defs;
    FunctionScope() noexcept: Scope(FUNCTION_SCOPE_CLASS) {}
  };

  static int trans(UNode &dest, Scope &scope, UNode &src);

  int trans(UNode &dest, UNode &src) {
    return trans(dest, rtol(Scope(Scope::GLOBAL_SCOPE_CLASS)), src);
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

  static inline UNode number(double d, const char *start = nullptr) {
    return UNode(new Number(start, d));
  }

  using Trans = int (UNode &dest, Scope &scope, const char *start, UNode &src);

  static Trans *isTrans(const UNode &src) noexcept;

  static int trans(UNodes &dest, Scope &scope, UNode &src);

  int trans1(UNode &dest, Scope &scope, UNode &src) {
    if (auto num = dynamic_cast<const NumberAtom *>(src.get()); num) {
      dest = number(num->value, num->start);
      return 0;
    }
    if (auto t = dynamic_cast<const TokenAtom *>(src.get()); t) {
      switch (t->token) {
        case "callee"_token: {
          if (scope.clazz == Scope::FUNCTION_SCOPE_CLASS) {
            dest.reset(new Callee(src->start));
          } else {
            dest.reset(new Null(src->start));
          }
          return 0;
        }
        default: {
          throw AstBad(bad::UNEXPECTED_TOKEN, src->start);
        }
      }
    }
    if (auto ls = dynamic_cast<List *>(src.get()); ls) {
      if (!ls->first) {
        dest.reset(new Null(src->start));
        return 0;
      }
      if (auto t = isTrans(ls->first); t) {
        return t(dest, scope, ls->start, ls->first->next);
      }
      {
        UNodes args;
        trans(args, scope, ls->first->next);
        UNode callee;
        trans1(callee, scope, ls->first);
        dest.reset(new Call(src->start, std::move(callee), std::move(args)));
      }
      return 0;
    }
    dest = std::move(src);
    return 0;
  }

  template<uint64_t>
  static Trans transSymbol;

  #define declTransSymbol(symb) \
  template<> \
  Trans transSymbol<symb##_token>

  #define declTransKeyword(kwd) declTransSymbol(#kwd)

  declTransKeyword(def);
  declTransKeyword(defer);
  declTransKeyword(forward);
  declTransKeyword(if);
  declTransKeyword(length);
  declTransKeyword(return);
  declTransKeyword(throw);
  declTransKeyword(try);
  declTransKeyword(yield);

  #undef declTransKeyword

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
    #define ifSymbol(symb) \
    if (a->token == symb##_token) { \
      return transSymbol<symb##_token>; \
    }
    #define ifKeyword(kwd) ifSymbol(#kwd)
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
    return nullptr;
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

  template<>
  int transSymbol<"def"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    if (!src) [[unlikely]] {
      throw AstBad(bad::DEF_NOTHING, start);
    }
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw AstBad(bad::UNEXPECTED_TOKEN, src->start);
    }
    if (scope.clazz == Scope::FUNCTION_SCOPE_CLASS) {
      static_cast<FunctionScope &>(scope).defs.insert(id->name);
    }
    remove(src->next);
    dest = std::move(src);
    return 0;
  }

  template<class T>
  static inline int trans2(UNode &dest, Scope &scope, const char *start, UNode &src) {
    UNode a;
    if (src) {
      trans(a, scope, src);
    } else {
      a.reset(new Null);
    }
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  static inline int noGlobal(const Scope &scope, const char *start) {
    if (scope.clazz == Scope::GLOBAL_SCOPE_CLASS) {
      throw AstBad(bad::SHOULD_NOT_IN_GLOBAL, start);
    }
    return 0;
  }

  template<>
  int transSymbol<"defer"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    noGlobal(scope, start);
    return trans2<Defer>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"forward"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    noGlobal(scope, start);
    UNode callee;
    UNodes args;
    if (src) {
      trans(args, scope, src->next);
      trans1(callee, scope, src);
    } else {
      callee.reset(new Null);
    }
    dest.reset(new Forward(start, std::move(callee), std::move(args)));
    return 0;
  }

  template<>
  int transSymbol<"if"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
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
    dest.reset(new If(start, std::move(cond), std::move(then), std::move(elze)));
    return 0;
  }

  template<>
  int transSymbol<"length"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2<Operation1<1, "length"_token>>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"return"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    noGlobal(scope, start);
    return trans2<Return>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"throw"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2<Throw>(dest, scope, start, src);
  }

  static UNode &endOf(UNode &src) noexcept {
    return src ? endOf(src->next) : src;
  }

  template<>
  int transSymbol<"try"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    UNode body;
    trans(body, scope, src);
    endOf(body).reset(new Throw(nullptr, UNode(new Null)));
    dest.reset(new Try(start, std::move(body)));
    return 0;
  }

  template<>
  int transSymbol<"yield"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2<Yield>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"!"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2<LogicNotOper>(dest, scope, start, src);
  }

  template<class T>
  static inline int trans2a(UNode &dest, Scope &scope, const char *start, UNode &src, double d, double d1) {
    UNodes a;
    trans(a, scope, src);
    switch (a.size()) {
      case 0: {
        a.push_back(number(d));
        [[fallthrough]];
      }
      case 1: {
        a.push_back(number(d1));
        [[fallthrough]];
      }
    }
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<class T>
  static inline int trans2a(UNode &dest, Scope &scope, const char *start, UNode &src, double d = NAN) {
    return trans2a<T>(dest, scope, start, src, d, d);
  }

  template<>
  int transSymbol<"%"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<ArithModOper>(dest, scope, start, src);
  }

  template<class T>
  static inline int trans3(UNode &dest, Scope &scope, const char *start, UNode &src) {
    UNodes a;
    trans(a, scope, src);
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<"&&"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans3<LogicAndOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"&"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<BitwsAndOper>(dest, scope, start, src, 0);
  }

  template<>
  int transSymbol<"**"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<ArithPowOper>(dest, scope, start, src, NAN, 1);
  }

  template<>
  int transSymbol<"*"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<ArithMulOper>(dest, scope, start, src, NAN, 1);
  }

  template<>
  int transSymbol<"+"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<ArithAddOper>(dest, scope, start, src, 0);
  }

  template<>
  int transSymbol<","_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans(dest, scope, src);
  }

  template<>
  int transSymbol<"-"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    UNodes items;
    trans(items, scope, src);
    if (items.empty()) {
      dest = number(0);
      return 0;
    }
    if (items.size() == 1) {
      items.push_back(std::move(items[0]));
      items[0] = number(0);
    }
    dest.reset(new ArithSubOper(start, std::move(items)));
    return 0;
  }

  template<class T>
  static inline int trans4(UNode &dest, Scope &scope, const char *start, UNode &src) {
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
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<"."_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans4<GetMemberOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"/"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<ArithDivOper>(dest, scope, start, src, NAN, 1);
  }

  template<>
  int transSymbol<"<<"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<LshOper>(dest, scope, start, src, NAN, 0);
  }

  template<class T>
  static inline int trans5(UNode &dest, Scope &scope, const char *start, UNode &src) {
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
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<"<=>"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans5<CompareOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"<="_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans5<CmpLteqOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"<"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans5<CmpLtOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"=="_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans5<CmpEqOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"="_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    UNode a;
    UNode b;
    if (src && src->next) {
      trans(b, scope, src->next);
    } else {
      b.reset(new Null);
    }
    if (!src) {
      throw AstBad(bad::ASSIGN_NOTHING, start);
    }
    trans1(a, scope, src);
    if (dynamic_cast<const IDAtom *>(a.get())) {
      dest.reset(new AssignOper(start, { std::move(a), std::move(b) }));
    } else if (auto c = dynamic_cast<GetMemberOper *>(a.get()); c) {
      if (c->items.size() == 2) {
        dest.reset(new SetMemberOper(start, { std::move(c->items[0]), std::move(c->items[1]), std::move(b) }));
      } else {
        auto d = std::move(c->items.back());
        c->items.pop_back();
        dest.reset(new SetMemberOper(start, { std::move(a), std::move(d), std::move(b) }));
      }
    } else {
      throw AstBad(bad::ILLEGAL_LHS, a->start);
    }
    return 0;
  }

  template<>
  int transSymbol<">="_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans5<CmpGteqOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<">>>"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<UshOper>(dest, scope, start, src, 0);
  }

  template<>
  int transSymbol<">>"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<RshOper>(dest, scope, start, src, 0);
  }

  template<>
  int transSymbol<">"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans5<CmpGtOper>(dest, scope, start, src);
  }

  static int fnParams(vector<const string *> &dest, Scope &scope, const UNode &src);

  template<>
  int transSymbol<"@"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    auto ls = dynamic_cast<const List *>(src.get());
    if (!ls) {
      throw AstBad(bad::UNEXPECTED_TOKEN, start);
    }
    FunctionScope fnScope;
    Function::Params params;
    fnParams(params, fnScope, src);
    UNode body;
    trans(body, fnScope, src->next);
    endOf(body).reset(new Return(nullptr, UNode(new Null)));
    dest.reset(new Function(start, std::move(fnScope.defs), std::move(params), std::move(body)));
    return 0;
  }

  int fnParams(vector<const string *> &dest, Scope &scope, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(src.get()); id) {
      dest.push_back(id->name);
      static_cast<FunctionScope &>(scope).defs.insert(id->name);
      return fnParams(dest, scope, src->next);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls && !ls->first) {
      dest.push_back(nullptr);
      return fnParams(dest, scope, src->next);
    }
    throw AstBad(bad::ILLEGAL_FN_PARAM, src->start);
  }

  template<>
  int transSymbol<"^^"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans4<LogicXorOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"^"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<BitwsXorOper>(dest, scope, start, src, 0);
  }

  template<>
  int transSymbol<"||"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans3<LogicOrOper>(dest, scope, start, src);
  }

  template<>
  int transSymbol<"|"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2a<BitwsOrOper>(dest, scope, start, src, 0);
  }

  template<>
  int transSymbol<"~"_token>(UNode &dest, Scope &scope, const char *start, UNode &src) {
    return trans2<BitwsNotOper>(dest, scope, start, src);
  }
}
