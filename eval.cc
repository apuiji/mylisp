#include<algorithm>
#include<functional>
#include"direction.hh"
#include"eval.hh"
#include"gc.hh"
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

  using rte::itCoroutine;

  static int push() {
    auto &top = itCoroutine->valuekTop;
    if (top + 1 == itCoroutine->valuek->end()) {
      throw Bad("out of stack");
    }
    ++top;
    return 0;
  }

  static inline int pop() noexcept {
    --itCoroutine->valuekTop;
    return 0;
  }

  // arithmetical directions begin
  template<class F>
  static double arithmetical1(double d, const Value *it, const Value *end, F &f) noexcept {
    return it != end ? arithmetical1(f(d, (double) *it), it + 1, end, f) : d;
  }

  template<class F>
  static inline int arithmetical(const char *it, const char *end, F &&f) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->valuekTop;
    auto v = top - n;
    itCoroutine->value = arithmetical1((double) *v, v + 1, top, f);
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
    itCoroutine->value = !itCoroutine->value;
    return eval(it, end);
  }

  static bool x0r(bool b, const Value *it, const Value *end) noexcept {
    return it != end ? x0r(b ^ (bool) *it, it + 1, end) : b;
  }

  int evalXOR(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->valuekTop;
    auto v = top - n;
    itCoroutine->value = x0r(false, v, top);
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
    auto &top = itCoroutine->valuekTop;
    auto v = top - n;
    itCoroutine->value = bitwise1((int) *v, v + 1, top, f);
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
    itCoroutine->value = f(itCoroutine->valuekTop[-1], itCoroutine->value);
    pop();
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

  static int call(size_t argc);

  int evalCALL(const char *it, const char *end) {
    size_t argc = *(const size_t *) it;
    call(it + sizeof(size_t), end, argc);
    return eval(it + sizeof(size_t), end);
  }

  static int popDefers(size_t prevDeferkSize) noexcept;

  int call(size_t argc) {
    auto &top = itCoroutine->valuekTop;
    auto itArg = top - argc;
    auto callee = itArg - 1;
    if (NativeFunction *nf; dynamicast(nf, *callee)) {
      itCoroutine->value = nf(itArg, top);
      top = callee;
      return 0;
    }
    if (FunctionObj *fo; dynamicast(fo, *callee)) {
      auto prevValuekBottom = itCoroutine->valuekBottom;
      size_t prevDeferkSize = itCoroutine->deferk.size();
      itCoroutine->valuekBottom = itArg;
      itCoroutine->localDefsk.push_back({});
      try {
        eval(fo->body.data(), fo->body.data() + fo->body.size());
      } catch (Forward fwd) {
        copy(top - fwd.argc - 1, top, callee);
        itCoroutine->valuekBottom = prevValuekBottom;
        top = callee + 1 + fwd.argc;
        itCoroutine->localDefsk.pop_back();
        popDefers(prevDeferkSize);
        return call(fwd.argc);
      } catch (Return) {
        itCoroutine->valuekBottom = prevValuekBottom;
        top = callee;
        itCoroutine->localDefsk.pop_back();
        popDefers(prevDeferkSize);
        return 0;
      } catch (Throw t) {
        itCoroutine->valuekBottom = prevValuekBottom;
        top = callee;
        itCoroutine->localDefsk.pop_back();
        popDefers(prevDeferkSize);
        throw t;
      }
      // never
      return 0;
    }
    itCoroutine->value = false;
    top = callee;
    return eval(next, end);
  }

  static int popDefers1(size_t n) noexcept;

  int popDefers(size_t prevDeferkSize) noexcept {
    size_t n = itCoroutine->deferk.size() - prevDeferkSize;
    if (n) {
      push();
      popDefers1(n);
      itCoroutine->value = itCoroutine->valuekTop[-1];
      pop();
    }
    return 0;
  }

  int popDefers1(size_t n) noexcept {
    if (!n) [[unlikely]] {
      return 0;
    }
    itCoroutine->value = itCoroutine->deferk.back();
    itCoroutine->deferk.pop_back();
    push();
    try {
      call(0);
    } catch (...) {
    }
    return popDefers1(n - 1);
  }

  int evalCLN_ARGS(const char *it, const char *end) {
    itCoroutine->valuekTop = itCoroutine->valuekBottom;
    return eval(it, end);
  }

  int evalFORWARD(const char *it, const char *end) {
    size_t argc = *(const size_t *) it;
    throw Forward(argc);
  }

  int evalGET_ARG(const char *it, const char *end) {
    size_t i = *(const size_t *) it;
    auto arg = itCoroutine->valuekBottom + i;
    if (arg < itCoroutine->valuekTop) {
      itCoroutine->value = *arg;
    } else {
      itCoroutine->value = false;
    }
    return eval(it + sizeof(size_t), end);
  }

  int evalGET_CLOSURE(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    auto fo = (FunctionObj *) *itCoroutine->valuekBottom;
    itCoroutine->value = fo->closures[name];
    return eval(it + sizeof(void *), end);
  }

  int evalGET_GLOBAL(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    itCoroutine->value = globalDefs[name];
    return eval(it + sizeof(void *), end);
  }

  int evalGET_LOCAL(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    itCoroutine->value = itCoroutine->localDefsk.back()[name];
    return eval(it + sizeof(void *), end);
  }

  static int getMemb(Value &dest, const Value *it, const Value *end) noexcept;

  int evalGET_MEMB(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    auto &top = itCoroutine->valuek.top;
    itCoroutine->value = top[-n];
    getMemb(itCoroutine->value, top - n + 1, top);
    top -= n;
    return eval(it + sizeof(size_t), end);
  }

  int getMemb(Value &dest, const Value *it, const Value *end) noexcept {
    if (it == end) [[unlikely]] {
      return 0;
    }
    dest = dest[*it];
    return getMemb(dest, it + 1, end);
  }

  int evalGET_PTR(const char *it, const char *end) {
    itCoroutine->value = **(PointerObj *) itCoroutine->value;
    return eval(it, end);
  }

  int evalINPUT_CLOSURE(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    auto fo = (FunctionObj *) itCoroutine->valuekTop[-1];
    fo->closures[name] = itCoroutine->value;
    return eval(it + sizeof(void *), end);
  }

  int evalJIF(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    if (itCoroutine->value) {
      return eval(it + sizeof(size_t) + n, end);
    } else {
      return eval(it + sizeof(size_t), end);
    }
  }

  int evalJMP(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    return eval(it + sizeof(size_t) + n, end);
  }

  int evalMAKE_FN(const char *it, const char *end) {
    auto body = *(const string **) it;
    auto fo = new FunctionObj(*body);
    itCoroutine->value = fo;
    gc::neobj(fo);
    return eval(it + sizeof(void *), end);
  }

  int evalMAKE_PTR(const char *it, const char *end) {
    auto po = new PointerObj;
    itCoroutine->value = po;
    gc::neobj(po);
    return eval(it, end);
  }

  int evalPOP(const char *it, const char *end) {
    pop();
    return eval(it, end);
  }

  int evalPUSH(const char *it, const char *end) {
    push();
    return eval(it, end);
  }

  int evalPUSH_DEFER(const char *it, const char *end) {
    itCoroutine->deferk.push_back(itCoroutine->value);
    return eval(it, end);
  }

  int evalRETURN(const char *it, const char *end) {
    throw Return();
  }

  int evalSET_CALLEE(const char *it, const char *end) {
    itCoroutine->value = itCoroutine->valuekBottom[-1];
    return eval(it, end);
  }

  int evalSET_CHAR(const char *it, const char *end) {
    wchar_t c = *(const wchar_t *) it;
    itCoroutine->value = c;
    return eval(it + sizeof(wchar_t), end);
  }

  int evalSET_GLOBAL(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    globalDefs[name] = itCoroutine->value;
    return eval(it + sizeof(void *), end);
  }

  int evalSET_LATIN1(const char *it, const char *end) {
    auto s = *(const string **) it;
    itCoroutine->value = s;
    return eval(it + sizeof(void *), end);
  }

  int evalSET_LOCAL(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    itCoroutine->localDefsk.back()[name] = itCoroutine->value;
    return eval(it + sizeof(void *), end);
  }

  int evalSET_MEMB(const char *it, const char *end) {
    auto &top = itCoroutine->valuek.top;
    top[-2][top[-1]] = itCoroutine->value;
    top -= 2;
    return eval(it, end);
  }

  int evalSET_NULL(const char *it, const char *end) {
    itCoroutine->value = false;
    return eval(it, end);
  }

  int evalSET_NUM(const char *it, const char *end) {
    double d = *(const double *) it;
    itCoroutine->value = d;
    return eval(it + sizeof(double), end);
  }

  int evalSET_PTR(const char *it, const char *end) {
    auto po = (PointerObj *) itCoroutine->valuekTop[-1];
    **po = itCoroutine->value;
    pop();
    return eval(it, end);
  }

  int evalSET_STR(const char *it, const char *end) {
    auto s = *(const wstring **) it;
    itCoroutine->value = s;
    return eval(it + sizeof(void *), end);
  }

  int evalTHROW(const char *it, const char *end) {
    throw Throw();
  }

  int evalTRY(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    auto prevTop = itCoroutine->valuekTop;
    try {
      eval(it + sizeof(size_t), end);
    } catch (Throw) {
    }
    itCoroutine->valuekTop = prevTop;
    return eval(it + sizeof(size_t) + n, end);
  }

  int evalYIELD(const char *it, const char *end) {
    itCoroutine->sink();
    return eval(it, end);
  }
}
