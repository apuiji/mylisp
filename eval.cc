#include<functional>
#include"direction.hh"
#include"eval.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp {
  #define declEval(DIR) \
  static int eval##DIR(const char *it, const char *end)

  declEval(ADD);
  declEval(AND);

  #undef declEval

  int eval(const char *it, const char *end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    switch (*it) {
      #define ifDir(DIR) \
      case direction::DIR: { \
        return eval##DIR(it + 1, end); \
      }
      ifDir(ADD);
      #undef ifDir
      default: {
        return eval(it + 1, end);
      }
    }
  }

  // arithmetical directions begin
  template<class F>
  static double arithmetical1(double d, const Value *it, const Value *end, F &f) noexcept {
    return it != end ? arithmetical1(f(d, (double) *it), it + 1, end, f) : d;
  }

  template<class F>
  static inline int arithmetical(const char *it, const char *end, F &&f) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->stack.top;
    auto v = top - n;
    *v = arithmetical1((double) *v, v + 1, top, f);
    top = v;
    return eval(it + sizeof(size_t), end);
  }

  int evalADD(const char *it, const char *end) {
    return arithmetical(it, end, plus<double>());
  }

  int evalSUB(const char *it, const char *end) {
    return arithmetical(it, end, minus<double>());
  }

  int evalMUL(const char *it, const char *end) {
    return arithmetical(it, end, multiplies<double>());
  }

  int evalDIV(const char *it, const char *end) {
    return arithmetical(it, end, divides<double>());
  }

  int evalMOD(const char *it, const char *end) {
    return arithmetical(it, end, ofr<double, double, double>(fmod));
  }

  int evalPOW(const char *it, const char *end) {
    return arithmetical(it, end, ofr<double, double, double>(pow));
  }
  // arithmetical directions end

  // logical directions begin
  int evalNOT(const char *it, const char *end) {
    *itCoroutine->stack.top = !*itCoroutine->stack.top;
    return eval(it, end);
  }

  static bool x0r(bool b, const Value *it, const Value *end) noexcept {
    return it != end ? x0r(b ^ (bool) *it, it + 1, end) : b;
  }

  int evalXOR(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->stack.top;
    auto v = top - n;
    *v = x0r(false, v, top);
    top = v;
    return eval(it + sizeof(size_t), end);
  }
  // logical directions end

  // bitwise directions begin
  template<class F>
  static int bitwise1(int i, const Value *it, const Value *end, F &f) noexcept {
    return it != end ? bitwise1(f(i, (int) *it), it + 1, end, f) : i;
  }

  template<class F>
  static inline int bitwise(const char *it, const char *end, F &&f) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->stack.top;
    auto v = top - n;
    *v = bitwise1((int) *v, it + 1, end, f);
    top = v;
    return eval(it + sizeof(size_t), end);
  }

  int evalBIT_AND(const char *it, const char *end) {
    return bitwise(it, end, [] (int i, int j) { return i & j; });
  }

  int evalBIT_OR(const char *it, const char *end) {
    return bitwise(it, end, [] (int i, int j) { return i | j; });
  }

  int evalBIT_NOT(const char *it, const char *end) {
    *itCoroutine->stack.top = ~(int) *itCoroutine->stack.top;
    return eval(it, end);
  }

  int evalBIT_XOR(const char *it, const char *end) {
    return bitwise(it, end, [] (int i, int j) { return i ^ j; });
  }

  int evalLSH(const char *it, const char *end) {
    return bitwise(it, end, [] (int i, int j) { return i << j; });
  }

  int evalRSH(const char *it, const char *end) {
    return bitwise(it, end, [] (int i, int j) { return i >> j; });
  }

  int evalUSH(const char *it, const char *end) {
    return bitwise(it, end, [] (int i, int j) { return (unsigned) i >> (unsigned) j; });
  }
  // bitwise directions end

  // compare directions begin
  int evalEQ(const char *it, const char *end) {
    auto &top = itCoroutine->stack.top;
    top[-1] = top[-1] == top[0];
    --top;
    return eval(it, end);
  }

  int evalLT(const char *it, const char *end) {
    ;
  }
  // compare directions end
}
