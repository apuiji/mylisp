#include<functional>
#include"direction.hh"
#include"eval.hh"
#include"object.hh"
#include"rte.hh"
#include"value.hh"

using namespace std;

namespace zlt::mylisp {
  #define declEval(DIR) \
  static int eval##DIR(const char *it, const char *end)

  declEval(ADD);
  declEval(BIT_AND);
  declEval(BIT_NOT);
  declEval(BIT_OR);
  declEval(BIT_XOR);
  declEval(CALL);
  declEval(CLN_ARGS);
  declEval(CMP);
  declEval(DIV);
  declEval(EQ);
  declEval(FORWARD);
  declEval(GET_ARG);
  declEval(GET_CLOSURE);
  declEval(GET_GLOBAL);
  declEval(GET_LOCAL);
  declEval(GET_MEMB);
  declEval(GET_PTR);
  declEval(GT);
  declEval(GTEQ);
  declEval(INPUT_CLOSURE);
  declEval(JIF);
  declEval(JMP);
  declEval(LSH);
  declEval(LT);
  declEval(LTEQ);
  declEval(MAKE_FN);
  declEval(MAKE_PTR);
  declEval(MOD);
  declEval(MUL);
  declEval(NOT);
  declEval(POP);
  declEval(POW);
  declEval(PUSH);
  declEval(PUSH_DEFER);
  declEval(RETURN);
  declEval(RSH);
  declEval(SET_CALLEE);
  declEval(SET_CHAR);
  declEval(SET_GLOBAL);
  declEval(SET_LATIN1);
  declEval(SET_LOCAL);
  declEval(SET_MEMB);
  declEval(SET_NULL);
  declEval(SET_NUM);
  declEval(SET_PTR);
  declEval(SET_STR);
  declEval(SUB);
  declEval(THROW);
  declEval(TRY);
  declEval(USH);
  declEval(XOR);
  declEval(YIELD);

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
      ifDir(BIT_AND);
      ifDir(BIT_NOT);
      ifDir(BIT_OR);
      ifDir(BIT_XOR);
      ifDir(CALL);
      ifDir(CLN_ARGS);
      ifDir(CMP);
      ifDir(DIV);
      ifDir(EQ);
      ifDir(FORWARD);
      ifDir(GET_ARG);
      ifDir(GET_CLOSURE);
      ifDir(GET_GLOBAL);
      ifDir(GET_LOCAL);
      ifDir(GET_MEMB);
      ifDir(GET_PTR);
      ifDir(GT);
      ifDir(GTEQ);
      ifDir(INPUT_CLOSURE);
      ifDir(JIF);
      ifDir(JMP);
      ifDir(LSH);
      ifDir(LT);
      ifDir(LTEQ);
      ifDir(MAKE_FN);
      ifDir(MAKE_PTR);
      ifDir(MOD);
      ifDir(MUL);
      ifDir(NOT);
      ifDir(POP);
      ifDir(POW);
      ifDir(PUSH);
      ifDir(PUSH_DEFER);
      ifDir(RETURN);
      ifDir(RSH);
      ifDir(SET_CALLEE);
      ifDir(SET_CHAR);
      ifDir(SET_GLOBAL);
      ifDir(SET_LATIN1);
      ifDir(SET_LOCAL);
      ifDir(SET_MEMB);
      ifDir(SET_NULL);
      ifDir(SET_NUM);
      ifDir(SET_PTR);
      ifDir(SET_STR);
      ifDir(SUB);
      ifDir(THROW);
      ifDir(TRY);
      ifDir(USH);
      ifDir(XOR);
      ifDir(YIELD);
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
    auto &top = itCoroutine->valuek.top;
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
    *itCoroutine->valuek.top = !*itCoroutine->valuek.top;
    return eval(it, end);
  }

  static bool x0r(bool b, const Value *it, const Value *end) noexcept {
    return it != end ? x0r(b ^ (bool) *it, it + 1, end) : b;
  }

  int evalXOR(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->valuek.top;
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
    auto &top = itCoroutine->valuek.top;
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
    *itCoroutine->valuek.top = ~(int) *itCoroutine->valuek.top;
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
  template<class F>
  static inline int compare(const char *it, const char *end, F &&f) {
    auto &top = itCoroutine->valuek.top;
    top[-1] = f(top[-1], top[0]);
    --top;
    return eval(it, end);
  }

  int evalEQ(const char *it, const char *end) {
    return compare(it, end, equal_to<Value>());
  }

  int evalLT(const char *it, const char *end) {
    return compare(it, end, less<Value>());
  }

  int evalGT(const char *it, const char *end) {
    return compare(it, end, greater<Value>());
  }

  int evalLTEQ(const char *it, const char *end) {
    return compare(it, end, less_equal<Value>());
  }

  int evalGTEQ(const char *it, const char *end) {
    return compare(it, end, greater_equal<Value>());
  }

  int evalCMP(const char *it, const char *end) {
    auto f = [] (const Value &a, const Value &b) -> Value {
      if (int diff; compare(diff, a, b)) {
        return diff;
      } else {
        return Null();
      }
    };
    return compare(it, end, f);
  }
  // compare directions end

  int evalCALL(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->valuek.top;
    auto itArg = top - n;
    auto callee = itArg - 1;
    if (auto nf = (NativeFunction *) *callee; nf) {
      *callee = nf(itArg, top);
      top = callee;
      return eval(it + sizeof(size_t), end);
    }
    if (auto fo = (FunctionObject *) *callee; fo) {
      auto prevBottom = itCoroutine->valuek.bottom;
      size_t prevLocalDefsn = itCoroutine->localDefsk.size();
      size_t prevDefern = itCoroutine->deferk.size();
      itCoroutine->valuek.bottom = itArg;
      itCoroutine->localDefsk.push_back({});
      try {
        *callee = eval(fo->body.data(), fo->body.data() + fo->body.size());
      } catch (Forward fwd) {
        ;
      } catch (Return) {
        ;
      } catch (Throw) {
        ;
      }
    }
  }

  declEval(CLN_ARGS);
  declEval(FORWARD);
  declEval(GET_ARG);
  declEval(GET_CLOSURE);
  declEval(GET_GLOBAL);
  declEval(GET_LOCAL);
  declEval(GET_MEMB);
  declEval(GET_PTR);
  declEval(INPUT_CLOSURE);
  declEval(JIF);
  declEval(JMP);
  declEval(MAKE_FN);
  declEval(MAKE_PTR);
  declEval(POP);
  declEval(PUSH);
  declEval(PUSH_DEFER);
  declEval(RETURN);
  declEval(SET_CALLEE);
  declEval(SET_CHAR);
  declEval(SET_GLOBAL);
  declEval(SET_LATIN1);
  declEval(SET_LOCAL);
  declEval(SET_MEMB);
  declEval(SET_NULL);
  declEval(SET_NUM);
  declEval(SET_PTR);
  declEval(SET_STR);
  declEval(THROW);
  declEval(TRY);
  declEval(YIELD);
}
