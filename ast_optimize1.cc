#include"ast_nodes1.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = UNodes::iterator;

  #define declOptimize(T) \
  static int optimizeBody(UNodes &dest, It it, It end, T &src)

  template<AnyOf<CharAtom, IDAtom, StringAtom, Callee, Function, Null, Number> T>
  declOptimize(T);
  template<AnyOf<Forward, Return, Throw> T>
  declOptimize(T);
  declOptimize(If);
  declOptimize(SequenceOper);

  #undef declOptimize

  int optimizeBody(UNodes &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    #define ifType(T) \
    if (auto a = dynamic_cast<T *>(it->get()); a) { \
      return optimizeBody(dest, it, end, *a); \
    }
    ifType(CharAtom);
    ifType(IDAtom);
    ifType(StringAtom);
    ifType(Callee);
    ifType(Null);
    ifType(Number);
    ifType(Forward);
    ifType(Function);
    ifType(If);
    ifType(Return);
    ifType(SequenceOper);
    ifType(Throw);
    #undef ifType
    dest.push_back(std::move(*it));
    return optimizeBody(dest, ++it, end);
  }

  template<AnyOf<CharAtom, IDAtom, StringAtom, Callee, Function, Null, Number> T>
  int optimizeBody(UNodes &dest, It it, It end, T &src) {
    if (next(it) == end) [[unlikely]] {
      dest.push_back(std::move(*it));
      return 0;
    }
    return optimizeBody(dest, ++it, end);
  }

  template<AnyOf<Forward, Return, Throw> T>
  int optimizeBody(UNodes &dest, It it, It end, T &src) {
    dest.push_back(std::move(*it));
    return 0;
  }

  static bool isTerminated(const UNode &src) noexcept;

  int optimizeBody(UNodes &dest, It it, It end, If &src) {
    dest.push_back(std::move(*it));
    if (isTerminated(src.then) && isTerminated(src.elze)) {
      return 0;
    }
    return optimizeBody(dest, ++it, end);
  }

  bool isTerminated(const UNode &src) noexcept {
    if (Dynamicastable<Forward, Return, Throw> {}(src.get())) {
      return true;
    }
    if (auto a = dynamic_cast<const If *>(src.get()); a) {
      return isTerminated(a->then) && isTerminated(a->elze);
    }
    if (auto a = dynamic_cast<const SequenceOper *>(src.get()); a) {
      return isTerminated(a->items.back());
    }
    return false;
  }

  int optimizeBody(UNodes &dest, It it, It end, SequenceOper &src) {
    auto a = std::move(src.items);
    a.insert(a.end(), move_iterator(++it), move_iterator(end));
    return optimizeBody(dest, a.begin(), a.end());
  }
}
