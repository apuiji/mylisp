#include<cmath>
#include"ast_nodes1.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = UNodes::iterator;

  static int optimize(UNode &src);

  int optimize(It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    return optimize(++it, end);
  }

  #define declOptimize(T) \
  static int optimize(UNode &dest, T &src)

  declOptimize(Call);
  declOptimize(Defer);
  declOptimize(Forward);
  declOptimize(Function);
  declOptimize(If);
  declOptimize(Return);
  declOptimize(Throw);
  declOptimize(Try);
  declOptimize(Yield);
  // operations begin
  // arithmetical operations begin
  declOptimize(ArithAddOper);
  declOptimize(ArithSubOper);
  declOptimize(ArithMulOper);
  declOptimize(ArithDivOper);
  declOptimize(ArithModOper);
  declOptimize(ArithPowOper);
  // arithmetical operations end
  // logical operations begin
  declOptimize(LogicAndOper);
  declOptimize(LogicOrOper);
  declOptimize(LogicNotOper);
  declOptimize(LogicXorOper);
  // logical operations end
  // bitwise operations begin
  declOptimize(BitwsAndOper);
  declOptimize(BitwsOrOper);
  declOptimize(BitwsNotOper);
  declOptimize(BitwsXorOper);
  declOptimize(LshOper);
  declOptimize(RshOper);
  declOptimize(UshOper);
  // bitwise operations end
  declOptimize(LengthOper);
  declOptimize(SequenceOper);
  declOptimize(Operation<1>);
  declOptimize(Operation<-1>);
  template<int N>
  declOptimize(Operation<N>);
  // operations end
  #undef declOptimize

  int optimize(UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<T *>(src.get()); a) { \
      return optimize(src, *a); \
    }
    ifType(Call);
    ifType(Callee);
    ifType(Defer);
    ifType(Forward);
    ifType(Function);
    ifType(If);
    ifType(Null);
    ifType(Return);
    ifType(Throw);
    ifType(Try);
    ifType(Yield);
    // operations begin
    // arithmetical operations begin
    ifType(ArithAddOper);
    ifType(ArithSubOper);
    ifType(ArithMulOper);
    ifType(ArithDivOper);
    ifType(ArithModOper);
    ifType(ArithPowOper);
    // arithmetical operations end
    // logical operations begin
    ifType(LogicAndOper);
    ifType(LogicOrOper);
    ifType(LogicNotOper);
    ifType(LogicXorOper);
    // logical operations end
    // bitwise operations begin
    ifType(BitwsAndOper);
    ifType(BitwsOrOper);
    ifType(BitwsNotOper);
    ifType(BitwsXorOper);
    ifType(LshOper);
    ifType(RshOper);
    ifType(UshOper);
    // bitwise operations end
    // compare operations begin
    ifType(CmpEqOper);
    ifType(CmpLtOper);
    ifType(CmpGtOper);
    ifType(CmpLteqOper);
    ifType(CmpGteqOper);
    ifType(CompareOper);
    // compare operations end
    ifType(AssignOper);
    ifType(GetMemberOper);
    ifType(LengthOper);
    ifType(SequenceOper);
    ifType(SetMemberOper);
    // operations end
    return 0;
  }

  int optimize(UNode &dest, Call &src) {
    optimize(src.callee);
    optimize(src.args.begin(), src.args.end());
    return 0;
  }

  int optimize(UNode &dest, Defer &src) {
    optimize(src.item);
    return 0;
  }

  int optimize(UNode &dest, Forward &src) {
    optimize(src.callee);
    optimize(src.args.begin(), src.args.end());
    return 0;
  }

  int optimize(UNode &dest, Function &src) {
    auto body = std::move(src.body);
    optimize(body.begin(), body.end());
    optimizeBody(src.body, body.begin(), body.end());
    return 0;
  }

  static bool isBoolConst(bool &dest, const UNode &src) noexcept;

  int optimize(UNode &dest, If &src) {
    optimize(src.cond);
    optimize(src.then);
    optimize(src.elze);
    if (bool b; isBoolConst(b, src.cond)) {
      dest = std::move(b ? src.then : src.elze);
      return 0;
    }
    if (auto a = dynamic_cast<LogicNotOper *>(src.cond.get()); a) {
      src.cond = std::move(a->item);
      swap(src.then, src.elze);
      return 0;
    }
    return 0;
  }

  bool isBoolConst(bool &dest, const UNode &src) noexcept {
    if (Dynamicastable<Number, CharAtom, StringAtom, Function> {}(*src)) {
      dest = true;
      return true;
    }
    if (Dynamicastable<Null> {}(*src)) {
      dest = false;
      return true;
    }
    return false;
  }

  int optimize(UNode &dest, Return &src) {
    optimize(src.value);
    return 0;
  }

  int optimize(UNode &dest, Throw &src) {
    optimize(src.value);
    return 0;
  }

  int optimize(UNode &dest, Try &src) {
    auto body = std::move(src.body);
    optimize(body.begin(), body.end());
    optimizeBody(src.body, body.begin(), body.end());
    return 0;
  }

  int optimize(UNode &dest, Yield &src) {
    optimize(src.then);
    return 0;
  }

  // operations begin
  // arithmetical operations begin
  template<class F>
  static int arithmetical(UNodes &dest, It it, It end, F &f);

  template<class T, class F>
  static inline int arithmetical(UNode &dest, T &src, F &&f) {
    UNodes items;
    arithmetical(items, src.items.begin(), src.items.end(), f);
    if (items.size() == 1) {
      dest = std::move(items[0]);
    } else {
      src.items = std::move(items);
    }
    return 0;
  }

  static bool isNumConst(double &dest, const UNode &src) noexcept;
  template<class F>
  static int arithmetical(UNodes &dest, double d, It it, It end, F &f);

  template<class F>
  int arithmetical(UNodes &dest, It it, It end, F &f) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    if (double d; isNumConst(d, *it)) {
      return arithmetical(dest, d, ++it, end, f);
    }
    dest.push_back(std::move(*it));
    return arithmetical(dest, ++it, end, f);
  }

  template<class F>
  int arithmetical(UNodes &dest, double d, It it, It end, F &f) {
    if (it == end) [[unlikely]] {
      UNode a(new Number(nullptr, d));
      dest.push_back(std::move(a));
      return 0;
    }
    optimize(*it);
    if (double d1; isNumConst(d1, *it)) {
      return arithmetical(dest, f(d, d1), ++it, end, f);
    }
    UNode a(new Number(nullptr, d));
    dest.push_back(std::move(a));
    dest.push_back(std::move(*it));
    return arithmetical(dest, ++it, end, f);
  }

  bool isNumConst(double &dest, const UNode &src) noexcept {
    if (auto a = dynamic_cast<const Number *>(src.get()); a) {
      dest = a->value;
      return true;
    }
    if (Dynamicastable<CharAtom, StringAtom, Callee, Function, Null> {}(*src)) {
      dest = NAN;
      return true;
    }
    return false;
  }

  int optimize(UNode &dest, ArithAddOper &src) {
    return arithmetical(dest, src, plus<double>());
  }

  int optimize(UNode &dest, ArithSubOper &src) {
    return arithmetical(dest, src, minus<double>());
  }

  int optimize(UNode &dest, ArithMulOper &src) {
    return arithmetical(dest, src, multiplies<double>());
  }

  int optimize(UNode &dest, ArithDivOper &src) {
    return arithmetical(dest, src, divides<double>());
  }

  int optimize(UNode &dest, ArithModOper &src) {
    return arithmetical(dest, src, ofr<double, double, double>(fmod));
  }

  int optimize(UNode &dest, ArithPowOper &src) {
    return arithmetical(dest, src, ofr<double, double, double>(pow));
  }
  // arithmetical operations end

  // logical operations begin
  static int logicAnd(UNodes &dest, UNode &last, It it, It end) {
    if (it == end) [[unlikely]] {
      dest.push_back(std::move(last));
      return 0;
    }
    optimize(*it);
    if (bool b; isBoolConst(b, last)) {
      return b ? logicAnd(dest, *it, next(it), end) : logicAnd(dest, last, end, end);
    }
    dest.push_back(std::move(last));
    return logicAnd(dest, *it, next(it), end);
  }

  static int logicOr(UNodes &dest, UNode &last, It it, It end) {
    if (it == end) [[unlikely]] {
      dest.push_back(std::move(last));
      return 0;
    }
    optimize(*it);
    if (bool b; isBoolConst(b, last)) {
      return b ? logicOr(dest, last, end, end) : logicOr(dest, *it, next(it), end);
    }
    dest.push_back(std::move(last));
    return logicOr(dest, *it, next(it), end);
  }

  template<class T>
  inline int logicAndOrLogicOr(UNode &dest, T &src) {
    if (src.items.empty()) [[unlikely]] {
      dest.reset(new Null);
      return 0;
    }
    UNodes items;
    It it = src.items.begin();
    It end = src.items.end();
    optimize(*it);
    if constexpr (is_same_v<T, LogicAndOper>) {
      logicAnd(items, *it, next(it), end);
    } else {
      logicOr(items, *it, next(it), end);
    }
    if (items.size() == 1) {
      dest = std::move(items[0]);
    } else {
      src.items = std::move(items);
    }
    return 0;
  }

  int optimize(UNode &dest, LogicAndOper &src) {
    return logicAndOrLogicOr(dest, src);
  }

  int optimize(UNode &dest, LogicOrOper &src) {
    return logicAndOrLogicOr(dest, src);
  }

  template<bool B>
  static inline int boo1(UNode &dest) {
    if constexpr (B) {
      dest.reset(new Number(nullptr, 1));
    } else {
      dest.reset(new Null);
    }
    return 0;
  }

  static inline int boo1(UNode &dest, bool b) {
    return b ? boo1<true>(dest) : boo1<false>(dest);
  }

  int optimize(UNode &dest, LogicNotOper &src) {
    optimize(src.item);
    if (bool b; isBoolConst(b, src.item)) {
      boo1(dest, !b);
    }
    return 0;
  }

  static int logicXor(UNodes &dest, It it, It end);

  int optimize(UNode &dest, LogicXorOper &src) {
    if (src.items.empty()) {
      dest.reset(new Null);
      return 0;
    }
    UNodes items;
    logicXor(items, src.items.begin(), src.items.end());
    if (items.size() == 1) {
      bool b;
      isBoolConst(b, items[0]);
      boo1(dest, b);
    } else {
      src.items = std::move(items);
    }
    return 0;
  }

  static int logicXor(UNodes &dest, bool b, It it, It end);

  int logicXor(UNodes &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    if (bool b; isBoolConst(b, *it)) {
      return logicXor(dest, b, ++it, end);
    }
    dest.push_back(std::move(*it));
    return logicXor(dest, ++it, end);
  }

  int logicXor(UNodes &dest, bool b, It it, It end) {
    if (it == end) [[unlikely]] {
      UNode a;
      boo1(a, b);
      dest.push_back(std::move(a));
      return 0;
    }
    optimize(*it);
    if (bool b1; isBoolConst(b1, *it)) {
      return logicXor(dest, b ^ b1, ++it, end);
    }
    UNode a;
    boo1(a, b);
    dest.push_back(std::move(a));
    dest.push_back(std::move(*it));
    return logicXor(dest, ++it, end);
  }
  // logical operations end

  // bitwise operations begin
  template<class F>
  static int bitwise(UNodes &dest, It it, It end, F &f);

  template<class T, class F>
  static inline int bitwise(UNode &dest, T &src, F &&f) {
    UNodes items;
    bitwise(items, src.items.begin(), src.items.end(), f);
    if (items.size() == 1) {
      dest = std::move(items[0]);
    } else {
      src.items = std::move(items);
    }
    return 0;
  }

  static inline bool isIntConst(int &dest, const UNode &src) noexcept {
    if (double d; isNumConst(d, src)) {
      dest = (int) d;
      return true;
    } else {
      return false;
    }
  }

  template<class F>
  static int bitwise(UNodes &dest, int i, It it, It end, F &f);

  template<class F>
  int bitwise(UNodes &dest, It it, It end, F &f) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    if (int i; isIntConst(i, *it)) {
      return bitwise(dest, i, ++it, end, f);
    }
    dest.push_back(std::move(*it));
    return bitwise(dest, ++it, end, f);
  }

  template<class F>
  int bitwise(UNodes &dest, int i, It it, It end, F &f) {
    if (it == end) [[unlikely]] {
      UNode a(new Number(nullptr, i));
      dest.push_back(std::move(a));
      return 0;
    }
    optimize(*it);
    if (int j; isIntConst(j, *it)) {
      return bitwise(dest, f(i, j), ++it, end, f);
    }
    UNode a(new Number(nullptr, i));
    dest.push_back(std::move(a));
    dest.push_back(std::move(*it));
    return bitwise(dest, ++it, end, f);
  }

  int optimize(UNode &dest, BitwsAndOper &src) {
    return bitwise(dest, src, [] (int i, int j) { return i & j; });
  }

  int optimize(UNode &dest, BitwsOrOper &src) {
    return bitwise(dest, src, [] (int i, int j) { return i | j; });
  }

  int optimize(UNode &dest, BitwsNotOper &src) {
    optimize(src.item);
    if (int i; isIntConst(i, src.item)) {
      dest.reset(new Number(nullptr, ~i));
    }
    return 0;
  }

  int optimize(UNode &dest, BitwsXorOper &src) {
    return bitwise(dest, src, [] (int i, int j) { return i ^ j; });
  }

  int optimize(UNode &dest, LshOper &src) {
    return bitwise(dest, src, [] (int i, int j) { return i << j; });
  }

  int optimize(UNode &dest, RshOper &src) {
    return bitwise(dest, src, [] (int i, int j) { return i >> j; });
  }

  int optimize(UNode &dest, UshOper &src) {
    return bitwise(dest, src, [] (int i, int j) { return (unsigned) i >> (unsigned) j; });
  }
  // bitwise operations end

  int optimize(UNode &dest, LengthOper &src) {
    optimize(src.item);
    if (auto a = dynamic_cast<const CharAtom *>(src.item.get()); a) {
      dest.reset(new Number(nullptr, 1));
      return 0;
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.item.get()); a) {
      dest.reset(new Number(nullptr, a->value->size()));
      return 0;
    }
    return 0;
  }

  int optimize(UNode &dest, SequenceOper &src) {
    auto items = std::move(src.items);
    optimize(items.begin(), items.end());
    optimizeBody(src.items, items.begin(), items.end());
    if (src.items.size() == 1) {
      dest = std::move(src.items[0]);
    }
    return 0;
  }

  int optimize(UNode &dest, Operation<1> &src) {
    optimize(src.item);
    return 0;
  }

  int optimize(UNode &dest, Operation<-1> &src) {
    optimize(src.items.begin(), src.items.end());
    return 0;
  }

  template<size_t N, size_t ...I>
  static inline int operationN(Operation<N> &src, index_sequence<I...>) {
    (optimize(src.items[I]), ...);
    return 0;
  }

  template<int N>
  int optimize(UNode &dest, Operation<N> &src) {
    return operationN(src, make_index_sequence<N>());
  }
  // operations end
}
