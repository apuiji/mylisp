#include"ast_trans2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using Indefs = set<const string *>;

  static int trans(const Indefs &indefs, UNode &src);

  int trans2(UNode &src) {
    return trans(Indefs(), src);
  }

  static int trans1(const Indefs &indefs, UNode &src);

  int trans(const Indefs &indefs, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    trans1(indefs, src);
    return trans(indefs, src->next);
  }

  #define declTrans(T) \
  static int trans(UNode &dest, const Indefs &indefs, T &src)

  declTrans(Call);
  declTrans(Defer);
  declTrans(Forward);
  declTrans(If);
  declTrans(Return);
  declTrans(Throw);
  declTrans(Try);
  declTrans(Yield);
  declTrans(AssignOper);
  declTrans(Operation<1>);
  template<int N>
  declTrans(Operation<N>);
  declTrans(Function1);
  declTrans(Reference1);

  #undef declTrans

  int trans1(const Indefs &indefs, UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<T *>(src.get()); a) { \
      return trans(src, indefs, *a); \
    }
    ifType(Call);
    ifType(Defer);
    ifType(Forward);
    ifType(If);
    ifType(Return);
    ifType(Throw);
    ifType(Try);
    ifType(Yield);
    ifType(AssignOper);
    ifType(Operation<1>);
    ifType(Operation<2>);
    ifType(Operation<3>);
    ifType(Operation<-1>);
    ifType(Function1);
    ifType(Reference1);
    #undef ifType
    return 0;
  }

  template<class It>
  static int trans(const Indefs &indefs, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    trans(indefs, *it);
    return trans(indefs, it + 1, end);
  }

  int trans(UNode &dest, const Indefs &indefs, Call &src) {
    trans(indefs, src.callee);
    trans(indefs, src.args.begin(), src.args.end());
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Defer &src) {
    trans(indefs, src.item);
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Forward &src) {
    trans(indefs, src.callee);
    trans(indefs, src.args.begin(), src.args.end());
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, If &src) {
    trans(indefs, src.cond);
    trans(indefs, src.then);
    trans(indefs, src.elze);
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Return &src) {
    trans(indefs, src.value);
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Throw &src) {
    trans(indefs, src.value);
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Try &src) {
    trans(indefs, src.body);
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Yield &src) {
    trans(indefs, src.then);
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, AssignOper &src) {
    trans(indefs, src.items[0]);
    trans(indefs, src.items[1]);
    if (auto a = dynamic_cast<GetIndirectOper *>(src.items[0].get()); a) {
      array<UNode, 2> items;
      items[0] = std::move(a->item);
      items[1] = std::move(src.items[1]);
      replace(dest, UNode(new SetIndirectOper(src.pos, std::move(items))));
    }
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Operation<1> &src) {
    trans(indefs, src.item);
    return 0;
  }

  template<int N>
  int trans(UNode &dest, const Indefs &indefs, Operation<N> &src) {
    trans(indefs, src.items.begin(), src.items.end());
    return 0;
  }

  using ItClosure = map<const string *, Reference>::const_iterator;

  static int makeInputClosure(UNode &dest, ItClosure it, ItClosure end);

  using ItPtrDef = set<const string *>::const_iterator;

  static UNode &makeIndirect(UNode &dest, ItPtrDef it, ItPtrDef end);

  int trans(UNode &dest, const Indefs &indefs, Function1 &src) {
    UNode inputClosure;
    makeInputClosure(inputClosure, src.closureDefs.begin(), src.closureDefs.end());
    UNode body;
    auto &next = makeIndirect(body, src.indefs.begin(), src.indefs.end());
    trans(src.indefs, src.body);
    next = std::move(src.body);
    UNode a(new Function2(src.pos, std::move(body), std::move(inputClosure)));
    replace(dest, a);
    return 0;
  }

  int makeInputClosure(UNode &dest, ItClosure it, ItClosure end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    dest.reset(new InputClosure(it->first, it->second));
    return makeInputClosure(dest->next, ++it, end);
  }

  UNode &makeIndirect(UNode &dest, ItPtrDef it, ItPtrDef end) {
    if (it == end) [[unlikely]] {
      return dest;
    }
    {
      array<UNode, 2> items;
      Reference ref(Reference::LOCAL_SCOPE, *it);
      items[0].reset(new Reference1(nullptr, ref));
      items[1].reset(new MakeIndirect);
      dest.reset(new AssignOper(nullptr, std::move(items)));
    }
    return makeIndirect(dest->next, ++it, end);
  }

  static bool isIndirect(const Indefs &indefs, const Reference &src) noexcept;

  int trans(UNode &dest, const Indefs &indefs, Reference1 &src) {
    if (isIndirect(indefs, src)) {
      UNode a(new Reference1(src.pos, src));
      a.reset(new GetIndirectOper(src.pos, std::move(a)));
      replace(dest, a);
    }
    return 0;
  }

  bool isIndirect(const Indefs &indefs, const Reference &src) noexcept {
    switch (src.scope) {
      case Reference::LOCAL_SCOPE: {
        return indefs.find(src.name) != indefs.end();
      }
      case Reference::CLOSURE_SCOPE: {
        return true;
      }
      default: {
        return false;
      }
    }
  }
}
