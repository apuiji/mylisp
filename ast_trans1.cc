#include"ast_trans1.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  struct Scope {
    virtual ~Scope() = default;
    virtual Reference find(const wstring *name, bool cross) = 0;
  };

  struct GlobalScope final: Scope {
    Reference find(const wstring *name, bool cross) override;
  };

  struct FunctionScope final: Scope {
    Scope &parent;
    const set<const wstring *> &defs;
    set<const wstring *> ptrDefs;
    map<const wstring *, Reference> closureDefs;
    FunctionScope(Scope &parent, const set<const wstring *> &defs) noexcept: parent(parent), defs(defs) {}
    Reference find(const wstring *name, bool cross) override;
  };

  static int trans(Scope &scope, UNode &src);

  int trans1(UNode &src) {
    return trans(rtol(GlobalScope()), src);
  }

  Reference GlobalScope::find(const wstring *name, bool cross) {
    return Reference(Reference::GLOBAL_SCOPE, name);
  }

  Reference FunctionScope::find(const wstring *name, bool cross) {
    if (defs.find(name) != defs.end()) {
      if (cross) {
        ptrDefs.insert(name);
      }
      return Reference(Reference::LOCAL_SCOPE, name);
    }
    if (closureDefs.find(name) != closureDefs.end()) {
      return Reference(Reference::CLOSURE_SCOPE, name);
    }
    auto ref = parent.find(name, true);
    if (ref.scope == Reference::GLOBAL_SCOPE) {
      return ref;
    }
    closureDefs[name] = ref;
    return Reference(Reference::CLOSURE_SCOPE, name);
  }

  static int trans1(Scope &scope, UNode &src);

  int trans(Scope &scope, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    trans1(scope, src);
    return trans(scope, src->next);
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
    UNode a(new Reference1(src.pos, scope.find(src.name, false)));
    replace(dest, a);
    return 0;
  }

  template<class It>
  static int trans(Scope &scope, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    trans(scope, *it);
    return trans(scope, it + 1, end);
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

  using ItParam = vector<const wstring *>::const_iterator;

  static UNode &transParams(UNode &dest, size_t i, ItParam it, ItParam end);

  int trans(UNode &dest, Scope &scope, Function &src) {
    FunctionScope fs(scope, src.defs);
    UNode body;
    auto &next = transParams(body, 0, src.params.begin(), src.params.end());
    trans(fs, src.body);
    next = std::move(src.body);
    UNode a(new Function1(src.pos, std::move(src.defs), std::move(fs.ptrDefs), std::move(fs.closureDefs), std::move(body)));
    replace(dest, a);
    return 0;
  }

  UNode &transParams(UNode &dest, size_t i, ItParam it, ItParam end) {
    if (it == end) [[unlikely]] {
      if (i) {
        dest.reset(new CleanArguments);
        return dest->next;
      } else {
        return dest;
      }
    }
    if (*it) {
      {
        array<UNode, 2> a;
        a[0].reset(new Reference1(nullptr, Reference(Reference::LOCAL_SCOPE, *it)));
        a[1].reset(new Argument(i));
        dest.reset(new AssignOper(nullptr, std::move(a)));
      }
      return transParams(dest->next, i + 1, it + 1, end);
    } else {
      return transParams(dest, i + 1, it + 1, end);
    }
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
    trans(scope, src.body);
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

  template<int N>
  int trans(UNode &dest, Scope &scope, Operation<N> &src) {
    trans(scope, src.items.begin(), src.items.end());
    return 0;
  }
}
