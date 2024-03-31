#include<cmath>
#include"ast_nodes1.hh"
#include"ast_token.hh"
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

  using It = UNodes::iterator;

  static int trans(UNodes &dest, Scope &scope, It it, It end);

  int trans(UNodes &dest, It it, It end) {
    Scope gs(Scope::GLOBAL_SCOPE_CLASS);
    return trans(dest, gs, it, end);
  }

  static int trans1(UNode &dest, Scope &scope, UNode &src);

  int trans(UNodes &dest, Scope &scope, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    {
      UNode a;
      trans1(a, scope, *it);
      dest.push_back(std::move(a));
    }
    return trans(dest, scope, ++it, end);
  }

  using Trans = int (UNode &dest, Scope &scope, const char *start, It it, It end);

  static Trans *isTrans(const UNode &src) noexcept;

  int trans1(UNode &dest, Scope &scope, UNode &src) {
    if (auto num = dynamic_cast<const NumberAtom *>(src.get()); num) {
      dest.reset(new Number(num->start, num->value));
      return 0;
    }
    if (auto t = dynamic_cast<const TokenAtom *>(src.get()); t) {
      if (t->token == "callee"_token) {
        if (scope.clazz == Scope::FUNCTION_SCOPE_CLASS) {
          dest.reset(new Callee(src->start));
        } else {
          dest.reset(new Null(src->start));
        }
        return 0;
      } else {
        throw AstBad(bad::UNEXPECTED_TOKEN, src->start);
      }
    }
    if (auto ls = dynamic_cast<List *>(src.get()); ls) {
      if (ls->items.empty()) {
        dest.reset(new Null(src->start));
        return 0;
      }
      It it = ls->items.begin();
      It end = ls->items.end();
      if (auto t = isTrans(*it); t) {
        return t(dest, scope, ls->start, ++it, end);
      }
      UNode callee;
      trans1(callee, scope, *it);
      UNodes args;
      trans(args, scope, ++it, end);
      dest.reset(new Call(src->start, std::move(callee), std::move(args)));
      return 0;
    }
    dest = std::move(src);
    return 0;
  }

  template<int>
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

  template<>
  int transSymbol<"def"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      throw AstBad(bad::DEF_NOTHING, start);
    }
    auto id = dynamic_cast<const IDAtom *>(it->get());
    if (!id) {
      throw AstBad(bad::UNEXPECTED_TOKEN, (**it).start);
    }
    if (scope.clazz == Scope::FUNCTION_SCOPE_CLASS) {
      static_cast<FunctionScope &>(scope).defs.insert(id->name);
    }
    dest = std::move(*it);
    return 0;
  }

  static int transItem(UNode &dest, Scope &scope, It it, It end) {
    if (it == end) [[unlikely]] {
      dest.reset(new Null);
      return 0;
    }
    UNodes a;
    trans(a, scope, it, end);
    if (a.size() == 1) {
      dest = std::move(a.front());
      return 0;
    }
    auto start = a.front()->start;
    dest.reset(new SequenceOper(start, std::move(a)));
    return 0;
  }

  template<class T>
  static inline int trans2(UNode &dest, Scope &scope, const char *start, It it, It end) {
    UNode a;
    transItem(a, scope, it, end);
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
  int transSymbol<"defer"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    noGlobal(scope, start);
    return trans2<Defer>(dest, scope, start, it, end);
  }

  static It transItem1(UNode &dest, Scope &scope, It it, It end) {
    if (it == end) [[unlikely]] {
      dest.reset(new Null);
      return it;
    }
    trans1(dest, scope, *it);
    return ++it;
  }

  template<>
  int transSymbol<"forward"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    noGlobal(scope, start);
    UNode callee;
    It it1 = transItem1(callee, scope, it, end);
    UNodes args;
    trans(args, scope, it1, end);
    dest.reset(new Forward(start, std::move(callee), std::move(args)));
    return 0;
  }

  template<>
  int transSymbol<"if"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    UNode cond;
    It it1 = transItem1(cond, scope, it, end);
    UNode then;
    It it2 = transItem1(then, scope, it1, end);
    UNode elze;
    transItem(elze, scope, it2, end);
    dest.reset(new If(start, std::move(cond), std::move(then), std::move(elze)));
    return 0;
  }

  template<>
  int transSymbol<"length"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2<LengthOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"return"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    noGlobal(scope, start);
    return trans2<Return>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"throw"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2<Throw>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"try"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    UNodes body;
    trans(body, scope, it, end);
    UNode a(new Throw(nullptr, UNode(new Null)));
    body.push_back(std::move(a));
    dest.reset(new Try(start, std::move(body)));
    return 0;
  }

  template<>
  int transSymbol<"yield"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2<Yield>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"!"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2<LogicNotOper>(dest, scope, start, it, end);
  }

  static int trans2num(UNodes &dest, Scope &scope, It it, It end, double d, double d1) {
    trans(dest, scope, it, end);
    if (dest.empty()) {
      UNode a(new Number(nullptr, d));
      dest.push_back(std::move(a));
    }
    if (dest.size() == 1) {
      UNode a(new Number(nullptr, d1));
      dest.push_back(std::move(a));
    }
    return 0;
  }

  template<class T>
  static inline int trans2a(UNode &dest, Scope &scope, const char *start, It it, It end, double d, double d1) {
    UNodes a;
    trans2num(a, scope, it, end, d, d1);
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<class T>
  static inline int trans2a(UNode &dest, Scope &scope, const char *start, It it, It end, double d = NAN) {
    return trans2a<T>(dest, scope, start, it, end, d, d);
  }

  template<>
  int transSymbol<"%"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<ArithModOper>(dest, scope, start, it, end);
  }

  template<class T>
  static inline int trans3(UNode &dest, Scope &scope, const char *start, It it, It end) {
    UNodes a;
    trans(a, scope, it, end);
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<"&&"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans3<LogicAndOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"&"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<BitwsAndOper>(dest, scope, start, it, end, 0);
  }

  template<>
  int transSymbol<"**"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<ArithPowOper>(dest, scope, start, it, end, NAN, 1);
  }

  template<>
  int transSymbol<"*"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<ArithMulOper>(dest, scope, start, it, end, NAN, 1);
  }

  template<>
  int transSymbol<"+"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<ArithAddOper>(dest, scope, start, it, end, 0);
  }

  template<>
  int transSymbol<","_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans3<SequenceOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"-"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    UNodes items;
    trans(items, scope, it, end);
    if (items.empty()) {
      dest.reset(new Number(start, 0));
      return 0;
    }
    if (items.size() == 1) {
      items.push_back(std::move(items.front()));
      items.front().reset(new Number(nullptr, 0));
    }
    dest.reset(new ArithSubOper(start, std::move(items)));
    return 0;
  }

  static int trans2null(UNodes &dest, Scope &scope, It it, It end) {
    trans(dest, scope, it, end);
    if (dest.empty()) {
      UNode a(new Null);
      dest.push_back(std::move(a));
    }
    if (dest.size() == 1) {
      UNode a(new Null);
      dest.push_back(std::move(a));
    }
    return 0;
  }

  template<class T>
  static inline int trans4(UNode &dest, Scope &scope, const char *start, It it, It end) {
    UNodes a;
    trans2null(a, scope, it, end);
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<"."_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans4<GetMemberOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"/"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<ArithDivOper>(dest, scope, start, it, end, NAN, 1);
  }

  template<>
  int transSymbol<"<<"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<LshOper>(dest, scope, start, it, end, NAN, 0);
  }

  template<class T>
  static inline int trans5(UNode &dest, Scope &scope, const char *start, It it, It end) {
    array<UNode, 2> a;
    It it1 = transItem1(a.front(), scope, it, end);
    transItem(a[1], scope, it1, end);
    dest.reset(new T(start, std::move(a)));
    return 0;
  }

  template<>
  int transSymbol<"<=>"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans5<CompareOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"<="_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans5<CmpLteqOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"<"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans5<CmpLtOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"=="_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans5<CmpEqOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"="_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      throw AstBad(bad::ASSIGN_NOTHING, start);
    }
    UNode a;
    trans1(a, scope, *it);
    UNode b;
    transItem(b, scope, ++it, end);
    if (dynamic_cast<const IDAtom *>(a.get())) {
      dest.reset(new AssignOper(start, { std::move(a), std::move(b) }));
      return 0;
    }
    if (auto c = dynamic_cast<GetMemberOper *>(a.get()); c) {
      if (c->items.size() == 2) {
        It it1 = c->items.begin();
        auto &x = *it1;
        ++it1;
        auto &y = *it1;
        dest.reset(new SetMemberOper(start, { std::move(x), std::move(y), std::move(b) }));
      } else {
        auto d = std::move(c->items.back());
        c->items.pop_back();
        dest.reset(new SetMemberOper(start, { std::move(a), std::move(d), std::move(b) }));
      }
      return 0;
    }
    throw AstBad(bad::ILLEGAL_LHS, a->start);
  }

  template<>
  int transSymbol<">="_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans5<CmpGteqOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<">>>"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<UshOper>(dest, scope, start, it, end, 0);
  }

  template<>
  int transSymbol<">>"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<RshOper>(dest, scope, start, it, end, 0);
  }

  template<>
  int transSymbol<">"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans5<CmpGtOper>(dest, scope, start, it, end);
  }

  static int fnParams(Function::Params &dest, Scope &scope, It it, It end);

  template<>
  int transSymbol<"@"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    auto ls = dynamic_cast<List *>(it->get());
    if (!ls) {
      throw AstBad(bad::UNEXPECTED_TOKEN, start);
    }
    FunctionScope fnScope;
    Function::Params params;
    fnParams(params, fnScope, ls->items.begin(), ls->items.end());
    UNodes body;
    trans(body, fnScope, ++it, end);
    UNode a(new Return(nullptr, UNode(new Null)));
    body.push_back(std::move(a));
    dest.reset(new Function(start, std::move(fnScope.defs), std::move(params), std::move(body)));
    return 0;
  }

  int fnParams(Function::Params &dest, Scope &scope, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(it->get()); id) {
      dest.push_back(id->name);
      static_cast<FunctionScope &>(scope).defs.insert(id->name);
      return fnParams(dest, scope, ++it, end);
    }
    if (auto ls = dynamic_cast<const List *>(it->get()); ls && ls->items.empty()) {
      dest.push_back(nullptr);
      return fnParams(dest, scope, ++it, end);
    }
    throw AstBad(bad::ILLEGAL_FN_PARAM, (**it).start);
  }

  template<>
  int transSymbol<"^^"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans4<LogicXorOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"^"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<BitwsXorOper>(dest, scope, start, it, end, 0);
  }

  template<>
  int transSymbol<"||"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans3<LogicOrOper>(dest, scope, start, it, end);
  }

  template<>
  int transSymbol<"|"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2a<BitwsOrOper>(dest, scope, start, it, end, 0);
  }

  template<>
  int transSymbol<"~"_token>(UNode &dest, Scope &scope, const char *start, It it, It end) {
    return trans2<BitwsNotOper>(dest, scope, start, it, end);
  }
}
