#include"ast_trans2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using PtrDefs = set<const wstring *>;

  static int trans(const PtrDefs &ptrDefs, UNode &src);

  int trans2(UNode &src) {
    return trans(PtrDefs(), src);
  }

  static int trans1(const PtrDefs &ptrDefs, UNode &src);

  int trans(const PtrDefs &ptrDefs, UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    trans1(ptrDefs, src);
    return trans(ptrDefs, src->next);
  }

  #define declTrans(T) \
  static int trans(UNode &dest, const PtrDefs &ptrDefs, T &src)

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

  int trans1(const PtrDefs &ptrDefs, UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<T *>(src.get()); a) { \
      return trans(src, ptrDefs, *a); \
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
  static int trans(const PtrDefs &ptrDefs, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    trans(ptrDefs, *it);
    return trans(ptrDefs, it + 1, end);
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Call &src) {
    trans(ptrDefs, src.callee);
    trans(ptrDefs, src.args.begin(), src.args.end());
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Defer &src) {
    trans(ptrDefs, src.item);
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Forward &src) {
    trans(ptrDefs, src.callee);
    trans(ptrDefs, src.args.begin(), src.args.end());
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, If &src) {
    trans(ptrDefs, src.cond);
    trans(ptrDefs, src.then);
    trans(ptrDefs, src.elze);
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Return &src) {
    trans(ptrDefs, src.value);
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Throw &src) {
    trans(ptrDefs, src.value);
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Try &src) {
    trans(ptrDefs, src.body);
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Yield &src) {
    trans(ptrDefs, src.then);
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, AssignOper &src) {
    trans(ptrDefs, src.items[0]);
    trans(ptrDefs, src.items[1]);
    if (auto a = dynamic_cast<GetPointerOper *>(src.items[0].get()); a) {
      array<UNode, 2> items;
      items[0] = std::move(a->item);
      items[1] = std::move(src.items[1]);
      replace(dest, UNode(new SetPointerOper(src.pos, std::move(items))));
    }
    return 0;
  }

  int trans(UNode &dest, const PtrDefs &ptrDefs, Operation<1> &src) {
    trans(ptrDefs, src.item);
    return 0;
  }

  template<int N>
  int trans(UNode &dest, const PtrDefs &ptrDefs, Operation<N> &src) {
    trans(ptrDefs, src.items.begin(), src.items.end());
    return 0;
  }


  int trans(UNode &dest, const PtrDefs &ptrDefs, Function1 &src) {
    ;
  }
}
