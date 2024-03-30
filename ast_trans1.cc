#include"ast_nodes2.hh"
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
    Scope &parent;
    const Function::Defs &defs;
    Function::Defs indefs;
    Function1::ClosureDefs closureDefs;
    FunctionScope(Scope &parent, const Function::Defs &defs) noexcept:
    Scope(FUNCTION_SCOPE_CLASS), parent(parent), defs(defs) {}
  };

  using It = UNodes::iterator;

  static int trans(Scope &scope, It it, It end);

  int trans1(It it, It end) {
    Scope gs(Scope::GLOBAL_SCOPE_CLASS);
    return trans(gs, it, end);
  }

  static Reference findDef(FunctionScope &scope, const string *name, bool cross);

  static Reference findDef(Scope &scope, const string *name, bool cross) {
    if (scope.clazz == Scope::GLOBAL_SCOPE_CLASS) {
      return Reference(Reference::GLOBAL_SCOPE, name);
    } else {
      return findDef(static_cast<FunctionScope &>(scope), name, cross);
    }
  }

  Reference findDef(FunctionScope &scope, const string *name, bool cross) {
    if (scope.defs.find(name) != scope.defs.end()) {
      if (cross) {
        scope.indefs.insert(name);
      }
      return Reference(Reference::LOCAL_SCOPE, name);
    }
    if (scope.closureDefs.find(name) != scope.closureDefs.end()) {
      return Reference(Reference::CLOSURE_SCOPE, name);
    }
    auto ref = findDef(scope.parent, name, true);
    if (ref.scope == Reference::GLOBAL_SCOPE) {
      return ref;
    }
    scope.closureDefs[name] = ref;
    return Reference(Reference::CLOSURE_SCOPE, name);
  }

  static int trans1(Scope &scope, UNode &src);

  int trans(Scope &scope, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    trans1(scope, *it);
    return trans(scope, ++it, end);
  }

  #define declTrans(T) \
  static int trans(UNode &dest, Scope &scope, T &src)

  declTrans(IDAtom);
  declTrans(Call);
  declTrans(Defer);
  declTrans(Forward);
  declTrans(Function);
  declTrans(If);
  declTrans(Return);
  declTrans(Throw);
  declTrans(Try);
  declTrans(Yield);
  declTrans(Operation<1>);
  declTrans(Operation<-1>);
  template<int N>
  declTrans(Operation<N>);

  #undef declTrans

  int trans1(Scope &scope, UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<T *>(src.get()); a) { \
      trans(src, scope, *a); \
      return 0; \
    }
    ifType(IDAtom);
    ifType(Call);
    ifType(Defer);
    ifType(Forward);
    ifType(Function);
    ifType(If);
    ifType(Return);
    ifType(Throw);
    ifType(Try);
    ifType(Yield);
    ifType(Operation<1>);
    ifType(Operation<2>);
    ifType(Operation<3>);
    ifType(Operation<-1>);
    #undef ifType
    return 0;
  }

  int trans(UNode &dest, Scope &scope, IDAtom &src) {
    dest.reset(new Reference1(src.start, findDef(scope, src.name, false)));
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Call &src) {
    trans(scope, src.callee);
    trans(scope, src.args.begin(), src.args.end());
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Defer &src) {
    trans(scope, src.item);
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Forward &src) {
    trans(scope, src.callee);
    trans(scope, src.args.begin(), src.args.end());
    return 0;
  }

  using ItParam = Function::Params::const_iterator;

  static int transParams(UNodes &dest, size_t i, ItParam it, ItParam end);

  int trans(UNode &dest, Scope &scope, Function &src) {
    FunctionScope fs(scope, src.defs);
    UNodes body;
    if (src.params.size()) {
      transParams(body, 0, src.params.begin(), src.params.end());
      body.push_back(UNode(new CleanArguments));
    }
    trans(fs, src.body.begin(), src.body.end());
    body.insert(body.end(), move_iterator(src.body.begin()), move_iterator(src.body.end()));
    dest.reset(new Function1(src.start, std::move(fs.indefs), std::move(fs.closureDefs), std::move(body)));
    return 0;
  }

  int transParams(UNodes &dest, size_t i, ItParam it, ItParam end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (*it) {
      array<UNode, 2> a;
      a[0].reset(new Reference1(nullptr, Reference(Reference::LOCAL_SCOPE, *it)));
      a[1].reset(new Argument(i));
      UNode b(new AssignOper(nullptr, std::move(a)));
      dest.push_back(std::move(b));
    }
    return transParams(dest, i + 1, ++it, end);
  }

  int trans(UNode &dest, Scope &scope, If &src) {
    trans(scope, src.cond);
    trans(scope, src.then);
    trans(scope, src.elze);
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Return &src) {
    trans(scope, src.value);
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Throw &src) {
    trans(scope, src.value);
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Try &src) {
    trans(scope, src.body.begin(), src.body.end());
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Yield &src) {
    trans(scope, src.then);
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Operation<1> &src) {
    trans(scope, src.item);
    return 0;
  }

  int trans(UNode &dest, Scope &scope, Operation<-1> &src) {
    trans(scope, src.items.begin(), src.items.end());
    return 0;
  }

  template<size_t N, size_t ...I>
  static inline int trans(Scope &scope, Operation<N> &src, index_sequence<I...>) {
    (trans(scope, src.items[I]), ...);
    return 0;
  }

  template<int N>
  int trans(UNode &dest, Scope &scope, Operation<N> &src) {
    return trans(scope, src, make_index_sequence<N>());
  }
}
