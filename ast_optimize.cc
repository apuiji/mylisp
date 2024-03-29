#include<cmath>
#include"ast_optimize.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  #define declOptimize(T) \
  static int optimize(UNode &dest, T &src)

  template<AnyOf<Number, CharAtom, StringAtom, IDAtom, Callee, Null> T>
  declOptimize(T);
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
  declOptimize(Operation<1>);
  template<int N>
  declOptimize(Operation<N>);
  // operations end
  #undef declOptimize

  int optimize(UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<T *>(src.get()); a) { \
      return optimize(src, *a); \
    }
    // ast_parse.hh definitions begin
    ifType(Number);
    ifType(CharAtom);
    ifType(StringAtom);
    ifType(IDAtom);
    // ast_parse.hh definitions end
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
    ifType(SetMemberOper);
    // operations end
    return 0;
  }

  template<AnyOf<Number, CharAtom, StringAtom, IDAtom, Callee, Null> T>
  int optimize(UNode &dest, T &src) {
    if (!dest->next) {
      return 0;
    }
    shift(dest);
    return optimize(dest);
  }

  template<class It>
  static int optimize(It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    return optimize(it + 1, end);
  }

  int optimize(UNode &dest, Call &src) {
    optimize(src.callee);
    optimize(src.args.begin(), src.args.end());
    return optimize(dest->next);;
  }

  int optimize(UNode &dest, Defer &src) {
    optimize(src.item);
    return optimize(dest->next);
  }

  int optimize(UNode &dest, Forward &src) {
    optimize(src.callee);
    optimize(src.args.begin(), src.args.end());
    return 0;
  }

  static inline UNode nvll() {
    return UNode(new Null);
  }

  int optimize(UNode &dest, Function &src) {
    if (src.next) {
      shift(dest);
      return optimize(dest);
    }
    optimize(src.body);
    return optimize(dest->next);
  }

  static bool isBoolConst(bool &dest, const UNode &src) noexcept;
  static bool isTerminated(const UNode &src) noexcept;

  int optimize(UNode &dest, If &src) {
    optimize(src.cond);
    optimize(src.then);
    optimize(src.elze);
    if (bool b; isBoolConst(b, src.cond)) {
      replace(dest, b ? src.then : src.elze);
      return optimize(dest);
    }
    if (auto a = dynamic_cast<LogicNotOper *>(src.cond.get()); a) {
      replace(src.cond, a->item);
      swap(src.then, src.elze);
    }
    if (isTerminated(src.then) && isTerminated(src.elze)) {
      return 0;
    }
    return optimize(dest->next);
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

  bool isTerminated(const UNode &src) noexcept {
    if (src->next) {
      return isTerminated(src->next);
    }
    if (Dynamicastable<Forward, Return, Throw> {}(*src)) {
      return true;
    }
    if (auto a = dynamic_cast<const If *>(src.get()); a) {
      return isTerminated(a->then) && isTerminated(a->elze);
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
    optimize(src.body);
    return optimize(dest->next);
  }

  int optimize(UNode &dest, Yield &src) {
    optimize(src.then);
    return optimize(dest->next);
  }

  // operations begin
  // arithmetical operations begin
  template<class F>
  static int arithmetical(UNodes &dest, UNodes::iterator it, UNodes::iterator end, F &f);

  template<class T, class F>
  static inline int arithmetical(UNode &dest, T &src, F &&f) {
    UNodes items;
    arithmetical(items, src.items.begin(), src.items.end(), f);
    if (items.size() == 1) {
      replace(dest, items[0]);
      return optimize(dest);
    }
    src.items = std::move(items);
    return optimize(dest->next);
  }

  static bool isNumConst(double &dest, const UNode &src) noexcept;
  template<class F>
  static int arithmetical(UNodes &dest, double d, UNodes::iterator it, UNodes::iterator end, F &f);

  template<class F>
  int arithmetical(UNodes &dest, UNodes::iterator it, UNodes::iterator end, F &f) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    if (double d; isNumConst(d, *it)) {
      return arithmetical(dest, d, it + 1, end, f);
    }
    dest.push_back(std::move(*it));
    return arithmetical(dest, it + 1, end, f);
  }

  static inline UNode number(double d) {
    return UNode(new Number(nullptr, d));
  }

  template<class F>
  int arithmetical(UNodes &dest, double d, UNodes::iterator it, UNodes::iterator end, F &f) {
    if (it == end) [[unlikely]] {
      dest.push_back(number(d));
      return 0;
    }
    optimize(*it);
    if (double d1; isNumConst(d1, *it)) {
      return arithmetical(dest, f(d, d1), it + 1, end, f);
    }
    dest.push_back(number(d));
    dest.push_back(std::move(*it));
    return arithmetical(dest, it + 1, end, f);
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
  template<bool B>
  static inline int logical(UNodes &dest, UNodes::iterator it, UNodes::iterator end) {
    optimize(*it);
    if (it + 1 == end) [[unlikely]] {
      dest.push_back(std::move(*it));
      return 0;
    }
    if (bool b; isBoolConst(b, *it)) {
      if constexpr (B) {
        if (b) {
          return logical<B>(dest, it + 1, end);
        } else {
          dest.push_back(nvll());
          return 0;
        }
      } else {
        if (b) {
          dest.push_back(std::move(*it));
          return 0;
        } else {
          return logical<B>(dest, it + 1, end);
        }
      }
    }
    dest.push_back(std::move(*it));
    return logical<B>(dest, it + 1, end);
  }

  template<bool B, class T>
  static inline int logical(UNode &dest, T &src) {
    if (src.items.empty()) {
      replace(dest, nvll());
      return optimize(dest);
    }
    UNodes items;
    logical<B>(items, src.items.begin(), src.items.end());
    if (items.size() == 1) {
      replace(dest, items[0]);
      return optimize(dest);
    }
    src.items = std::move(items);
    return optimize(dest->next);
  }

  int optimize(UNode &dest, LogicAndOper &src) {
    return logical<true>(dest, src);
  }

  int optimize(UNode &dest, LogicOrOper &src) {
    return logical<false>(dest, src);
  }

  template<bool B>
  static inline UNode boo1() {
    if constexpr (B) {
      return number(1);
    } else {
      return nvll();
    }
  }

  static inline UNode boo1(bool b) {
    return b ? boo1<true>() : boo1<false>();
  }

  int optimize(UNode &dest, LogicNotOper &src) {
    optimize(src.item);
    if (bool b; isBoolConst(b, src.item)) {
      replace(dest, boo1(!b));
      return optimize(dest);
    }
    return optimize(dest->next);
  }

  static int logicXor(UNodes &dest, UNodes::iterator it, UNodes::iterator end);

  int optimize(UNode &dest, LogicXorOper &src) {
    if (src.items.empty()) {
      replace(dest, nvll());
      return 0;
    }
    UNodes items;
    logicXor(items, src.items.begin(), src.items.end());
    if (items.size() == 1) {
      bool b;
      isBoolConst(b, items[0]);
      replace(dest, boo1(b));
      return optimize(dest);
    }
    src.items = std::move(items);
    return optimize(dest->next);
  }

  static int logicXor(UNodes &dest, bool b, UNodes::iterator it, UNodes::iterator end);

  int logicXor(UNodes &dest, UNodes::iterator it, UNodes::iterator end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    if (bool b; isBoolConst(b, *it)) {
      return logicXor(dest, b, it + 1, end);
    }
    dest.push_back(std::move(*it));
    return logicXor(dest, it + 1, end);
  }

  int logicXor(UNodes &dest, bool b, UNodes::iterator it, UNodes::iterator end) {
    if (it == end) [[unlikely]] {
      dest.push_back(boo1(b));
      return 0;
    }
    optimize(*it);
    if (bool b1; isBoolConst(b1, *it)) {
      return logicXor(dest, b ^ b1, it + 1, end);
    }
    dest.push_back(boo1(b));
    dest.push_back(std::move(*it));
    return logicXor(dest, it + 1, end);
  }
  // logical operations end

  // bitwise operations begin
  template<class F>
  static int bitwise(UNodes &dest, UNodes::iterator it, UNodes::iterator end, F &f);

  template<class T, class F>
  static inline int bitwise(UNode &dest, T &src, F &&f) {
    UNodes items;
    bitwise(items, src.items.begin(), src.items.end(), f);
    if (items.size() == 1) {
      replace(dest, items[0]);
      return optimize(dest);
    }
    src.items = std::move(items);
    return optimize(dest->next);
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
  static int bitwise(UNodes &dest, int i, UNodes::iterator it, UNodes::iterator end, F &f);

  template<class F>
  int bitwise(UNodes &dest, UNodes::iterator it, UNodes::iterator end, F &f) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    optimize(*it);
    if (int i; isIntConst(i, *it)) {
      return bitwise(dest, i, it + 1, end, f);
    }
    dest.push_back(std::move(*it));
    return bitwise(dest, it + 1, end, f);
  }

  template<class F>
  int bitwise(UNodes &dest, int i, UNodes::iterator it, UNodes::iterator end, F &f) {
    if (it == end) [[unlikely]] {
      dest.push_back(number(i));
      return 0;
    }
    optimize(*it);
    if (int j; isIntConst(j, *it)) {
      return bitwise(dest, f(i, j), it + 1, end, f);
    }
    dest.push_back(number(i));
    dest.push_back(std::move(*it));
    return bitwise(dest, it + 1, end, f);
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
      replace(dest, number(~i));
      return optimize(dest);
    }
    return optimize(dest->next);
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
      replace(dest, number(1));
      return optimize(dest);
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.item.get()); a) {
      replace(dest, number(a->value->size()));
      return optimize(dest);
    }
    return optimize(dest->next);
  }

  int optimize(UNode &dest, Operation<1> &src) {
    optimize(src.item);
    return optimize(dest->next);
  }

  template<int N>
  int optimize(UNode &dest, Operation<N> &src) {
    optimize(src.items.begin(), src.items.end());
    return optimize(dest->next);
  }
  // operations end
}
