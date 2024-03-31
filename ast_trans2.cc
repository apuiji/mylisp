#include"ast_nodes3.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using Indefs = Function::Defs;
  using It = UNodes::iterator;

  static int trans(const Indefs &indefs, UNode &src);

  static inline int trans(const Indefs &indefs, It it, It end) {
    for (; it != end; ++it) {
      trans(indefs, *it);
    }
    return 0;
  }

  int trans2(It it, It end) {
    return trans(Indefs(), it, end);
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

  int trans(const Indefs &indefs, UNode &src) {
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
    trans(indefs, src.body.begin(), src.body.end());
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
      dest.reset(new SetIndirectOper(src.start, std::move(items)));
    }
    return 0;
  }

  int trans(UNode &dest, const Indefs &indefs, Operation<1> &src) {
    trans(indefs, src.item);
    return 0;
  }

  template<int N>
  int trans(UNode &dest, const Indefs &indefs, Operation<N> &src) {
    for (auto &a : src.items) {
      trans(indefs, a);
    }
    return 0;
  }

  using ItClosure = Function1::ClosureDefs::const_iterator;

  static int makeInputClosures(UNodes &dest, ItClosure it, ItClosure end);

  using ItPtrDef = set<const string *>::const_iterator;

  static int makeIndirects(UNodes &dest, ItPtrDef it, ItPtrDef end);

  int trans(UNode &dest, const Indefs &indefs, Function1 &src) {
    UNodes inputClosures;
    makeInputClosures(inputClosures, src.closureDefs.begin(), src.closureDefs.end());
    UNodes body;
    makeIndirects(body, src.indefs.begin(), src.indefs.end());
    trans(src.indefs, src.body.begin(), src.body.end());
    body.insert(body.end(), move_iterator(src.body.begin()), move_iterator(src.body.end()));
    dest.reset(new Function2(src.start, std::move(body), std::move(inputClosures)));
    return 0;
  }

  int makeInputClosures(UNodes &dest, ItClosure it, ItClosure end) {
    for (; it != end; ++it) {
      UNode a(new InputClosure(it->first, it->second));
      dest.push_back(std::move(a));
    }
    return 0;
  }

  int makeIndirects(UNodes &dest, ItPtrDef it, ItPtrDef end) {
    for (; it != end; ++it) {
      array<UNode, 2> items;
      Reference ref(Reference::LOCAL_SCOPE, *it);
      items[0].reset(new Reference1(nullptr, ref));
      items[1].reset(new MakeIndirect);
      UNode a(new AssignOper(nullptr, std::move(items)));
      dest.push_back(std::move(a));
    }
    return 0;
  }

  static bool isIndirect(const Indefs &indefs, const Reference &src) noexcept;

  int trans(UNode &dest, const Indefs &indefs, Reference1 &src) {
    if (isIndirect(indefs, src)) {
      UNode a(new Reference1(src.start, src));
      dest.reset(new GetIndirectOper(src.start, std::move(a)));
    }
    return 0;
  }

  bool isIndirect(const Indefs &indefs, const Reference &src) noexcept {
    if (src.scope == Reference::LOCAL_SCOPE) {
      return indefs.find(src.name) != indefs.end();
    }
    if (src.scope == Reference::CLOSURE_SCOPE) {
      return true;
    }
    return false;
  }
}
