#include<algorithm>
#include<functional>
#include"direction.hh"
#include"eval.hh"
#include"gc.hh"
#include"myccutils/xyz.hh"
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
  declEval(CONTINUE_FORWARD);
  declEval(CONTINUE_RETURN);
  declEval(CONTINUE_THROW);
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

  using rte::itCoroutine;

  int eval(const char *it, const char *end) {
    if (it == end) [[unlikely]] {
      itCoroutine->alive = false;
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
      ifDir(CONTINUE_FORWARD);
      ifDir(CONTINUE_RETURN);
      ifDir(CONTINUE_THROW);
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

  static int push() {
    auto &top = itCoroutine->valuekTop;
    if (top + 1 == itCoroutine->valuek->end()) {
      throw EvalBad(L"out of stack");
    }
    *top = itCoroutine->value;
    ++top;
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
    itCoroutine->value = ~(int) itCoroutine->value;
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
    --itCoroutine->valuekTop;
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
    auto f = [] (const Value &a, const Value &b) -> double {
      if (int diff; compare(diff, a, b)) {
        return diff;
      } else {
        return NAN;
      }
    };
    return compare(it, end, f);
  }
  // compare directions end

  static int call(const char *next, const char *end, size_t argc);

  int evalCALL(const char *it, const char *end) {
    size_t argc = *(const size_t *) it;
    return call(it + sizeof(size_t), end, argc);
  }

  int call(const char *next, const char *end, size_t argc) {
    auto &top = itCoroutine->valuekTop;
    auto itArg = top - argc;
    auto callee = itArg - 1;
    if (NativeFunction *nf; dynamicast(nf, *callee)) {
      itCoroutine->value = nf(itArg, top);
      top = callee;
      return eval(next, end);
    }
    if (FunctionObj *fo; dynamicast(fo, *callee)) {
      // callee 1stArg ... lastArg none
      //        ^                  ^
      //        bottom             top
      itCoroutine->framek.push_back(Frame(Frame::CALL_FN_FRAME_CLASS, next, end, itCoroutine->valuekBottom, callee));
      itCoroutine->valuekBottom = itArg;
      itCoroutine->localDefsk.push_back({});
      return eval(fo->body.data(), fo->body.data() + fo->body.size());
    }
    itCoroutine->value = false;
    top = callee;
    return 0;
  }

  using ItFrame = list<Frame>::reverse_iterator;

  struct Forward {};
  struct Return {};
  struct Throw {};

  static int popFrame(ItFrame it, Forward fwd);
  static int popFrame(ItFrame it, Return ret);
  static int popFrame(ItFrame it, Throw thr);

  template<class T>
  static inline int evalCONTINUE_T(const char *it, const char *end, T t) {
    auto itFrame = itCoroutine->framek.rbegin();
    auto lastFrame = itFrame++;
    itCoroutine->framek.erase(lastFrame.base());
    return popFrame(itFrame, t);
  }

  int evalCONTINUE_FORWARD(const char *it, const char *end) {
    return evalCONTINUE_T(it, end, Forward());
  }

  int evalCONTINUE_RETURN(const char *it, const char *end) {
    return evalCONTINUE_T(it, end, Return());
  }

  int evalCONTINUE_THROW(const char *it, const char *end) {
    return evalCONTINUE_T(it, end, Throw());
  }

  template<int FrameClass, uint8_t Dir>
  static inline int popDefer(ItFrame it) {
    it->clazz = FrameClass;
    static constexpr const char dirs[] = { Dir };
    it->prevNext = dirs;
    it->prevEnd = dirs + 1;
    it->prevValuekBottom = itCoroutine->valuekBottom;
    it->prevValuekTop = itCoroutine->valuekTop;
    itCoroutine->value = itCoroutine->deferk.back();
    itCoroutine->deferk.pop_back();
    push();
    return call(dirs, dirs + 1, 0);
  }

  int popFrame(ItFrame it, Forward fwd) {
    switch (it->clazz) {
      case Frame::CALL_FN_FRAME_CLASS: {
        auto next = it->prevNext;
        auto end = it->prevEnd;
        auto callee = it->prevValuekTop;
        itCoroutine->framek.erase(it.base());
        return call(next, end, itCoroutine->valuekTop - callee - 1);
      }
      case Frame::DEFER_FRAME_CLASS: {
        return popDefer<Frame::CONTINUE_FORWARD_FRAME_CLASS, direction::CONTINUE_FORWARD>(it);
      }
      case Frame::TRY_FRAME_CLASS: {
        auto last = it++;
        itCoroutine->framek.erase(last.base());
        return popFrame(it, fwd);
      }
      default: {
        // never
        return 0;
      }
    }
  }

  int popFrame(ItFrame it, Return ret) {
    switch (it->clazz) {
      case Frame::CALL_FN_FRAME_CLASS: {
        auto next = it->prevNext;
        auto end = it->prevEnd;
        itCoroutine->value = *--itCoroutine->valuekTop;
        itCoroutine->framek.erase(it.base());
        return eval(next, end);
      }
      case Frame::DEFER_FRAME_CLASS: {
        return popDefer<Frame::CONTINUE_RETURN_FRAME_CLASS, direction::CONTINUE_RETURN>(it);
      }
      case Frame::TRY_FRAME_CLASS: {
        auto last = it++;
        itCoroutine->framek.erase(last.base());
        return popFrame(it, ret);
      }
      default: {
        // never
        return 0;
      }
    }
  }

  int popFrame(ItFrame it, Throw thr) {
    switch (it->clazz) {
      case Frame::CALL_FN_FRAME_CLASS: {
        itCoroutine->localDefsk.pop_back();
        auto last = it++;
        itCoroutine->framek.erase(last.base());
        return popFrame(it, thr);
      }
      case Frame::DEFER_FRAME_CLASS: {
        return popDefer<Frame::CONTINUE_THROW_FRAME_CLASS, direction::CONTINUE_THROW>(it);
      }
      default: {
        auto next = it->prevNext;
        auto end = it->prevEnd;
        itCoroutine->value = *--itCoroutine->valuekTop;
        itCoroutine->framek.erase(it.base());
        return eval(next, end);
      }
    }
  }

  int evalCLN_ARGS(const char *it, const char *end) {
    itCoroutine->valuekTop = itCoroutine->valuekBottom;
    return eval(it, end);
  }

  template<int ...Classes>
  struct FindFrame {
    ItFrame operator ()(ItFrame it) noexcept {
      bool b = ((it->clazz == Classes) || ...);
      return b ? it : operator ()(++it);
    }
  };

  int evalFORWARD(const char *it, const char *end) {
    size_t argc = *(const size_t *) it;
    auto itFrame = FindFrame<Frame::CALL_FN_FRAME_CLASS> {}(itCoroutine->framek.rbegin());
    copy(itCoroutine->valuekTop - argc - 1, itCoroutine->valuekTop, itFrame->prevValuekTop);
    itCoroutine->valuekBottom = itFrame->prevValuekBottom;
    itCoroutine->valuekTop = itFrame->prevValuekTop + 1 + argc;
    itCoroutine->localDefsk.pop_back();
    return popFrame(itCoroutine->framek.rbegin(), Forward());
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
    FunctionObj *fo;
    staticast(fo, *itCoroutine->valuekBottom);
    itCoroutine->value = fo->closures[name];
    return eval(it + sizeof(void *), end);
  }

  int evalGET_GLOBAL(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    itCoroutine->value = rte::globalDefs[name];
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
    auto &top = itCoroutine->valuekTop;
    itCoroutine->value = top[-n];
    getMemb(itCoroutine->value, top - n + 1, top);
    top -= n;
    return eval(it + sizeof(size_t), end);
  }

  int getMemb(Value &dest, const Value *it, const Value *end) noexcept {
    if (it == end) [[unlikely]] {
      return 0;
    }
    dest = getMemb(dest, *it);
    return getMemb(dest, it + 1, end);
  }

  int evalGET_PTR(const char *it, const char *end) {
    PointerObj *po;
    staticast(po, itCoroutine->value);
    itCoroutine->value = **po;
    return eval(it, end);
  }

  int evalINPUT_CLOSURE(const char *it, const char *end) {
    auto name = *(const wstring **) it;
    FunctionObj *fo;
    staticast(fo, itCoroutine->valuekTop[-1]);
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
    itCoroutine->value = neobj<FunctionObj>(*body);
    return eval(it + sizeof(void *), end);
  }

  int evalMAKE_PTR(const char *it, const char *end) {
    itCoroutine->value = neobj<PointerObj>();
    return eval(it, end);
  }

  int evalPOP(const char *it, const char *end) {
    --itCoroutine->valuekTop;
    return eval(it, end);
  }

  int evalPUSH(const char *it, const char *end) {
    push();
    return eval(it, end);
  }

  int evalPUSH_DEFER(const char *it, const char *end) {
    itCoroutine->deferk.push_back(itCoroutine->value);
    itCoroutine->framek.push_back(Frame(Frame::DEFER_FRAME_CLASS, nullptr, nullptr, nullptr, nullptr));
    return eval(it, end);
  }

  int evalRETURN(const char *it, const char *end) {
    auto itFrame = FindFrame<Frame::CALL_FN_FRAME_CLASS> {}(itCoroutine->framek.rbegin());
    itCoroutine->valuekBottom = itFrame->prevValuekBottom;
    itCoroutine->valuekTop = itFrame->prevValuekTop;
    push();
    itCoroutine->localDefsk.pop_back();
    return popFrame(itCoroutine->framek.rbegin(), Return());
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
    rte::globalDefs[name] = itCoroutine->value;
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
    auto &top = itCoroutine->valuekTop;
    setMemb(top[-2], top[-1], itCoroutine->value);
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
    PointerObj *po;
    staticast(po, itCoroutine->valuekTop[-1]);
    **po = itCoroutine->value;
    --itCoroutine->valuekTop;
    return eval(it, end);
  }

  int evalSET_STR(const char *it, const char *end) {
    auto s = *(const wstring **) it;
    itCoroutine->value = s;
    return eval(it + sizeof(void *), end);
  }

  int evalTHROW(const char *it, const char *end) {
    using FF = FindFrame<
      Frame::CONTINUE_FORWARD_FRAME_CLASS,
      Frame::CONTINUE_RETURN_FRAME_CLASS,
      Frame::CONTINUE_THROW_FRAME_CLASS,
      Frame::TRY_FRAME_CLASS>;
    auto itFrame = FF {}(itCoroutine->framek.rbegin());
    itCoroutine->valuekBottom = itFrame->prevValuekBottom;
    itCoroutine->valuekTop = itFrame->prevValuekTop;
    push();
    return popFrame(itCoroutine->framek.rbegin(), Throw());
  }

  int evalTRY(const char *it, const char *end) {
    size_t n = *(const size_t *) it;
    Frame f(Frame::TRY_FRAME_CLASS, it + sizeof(size_t) + n, end, itCoroutine->valuekBottom, itCoroutine->valuekTop);
    itCoroutine->framek.push_back(f);
    return eval(it + sizeof(size_t), end);
  }

  int evalYIELD(const char *it, const char *end) {
    itCoroutine->framek.push_back(Frame(0, it, end, nullptr, nullptr));
    return rte::yield();
  }
}
