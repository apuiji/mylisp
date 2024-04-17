#include<algorithm>
#include<cmath>
#include"gc.hh"
#include"mylisp.hh"
#include"object.hh"
#include"opcode.hh"
#include"value.hh"

namespace zlt::mylisp {
  struct CleanAllDeferBody {
    char value[256];
    CleanAllDeferBody();
  };

  struct CleanFnDeferBody {
    char value[256];
    CleanAllDeferBody();
  };

  static void call(size_t argc);
  static void yield();

  void exec() {
    using namespace it_coroutine;
    int op = *pc()++;
    if (op == opcode::ADD) {
      staticast<double>(peek()) += (double) ax();
    } else if (op == opcode::BIT_AND) {
      staticast<double>(peek()) = staticast<int>(peek()) & (int) ax();
    } else if (op == opcode::BIT_NOT) {
      ax() = ~(int) ax();
    } else if (op == opcode::BIT_OR) {
      staticast<double>(peek()) = staticast<int>(peek()) | (int) ax();
    } else if (op == opcode::BIT_XOR) {
      staticast<double>(peek()) = staticast<int>(peek()) ^ (int) ax();
    } else if (op == opcode::CALL) {
      size_t argc = consume<size_t>();
      call(argc);
      return;
    } else if (op == opcode::CHAR_LITERAL) {
      ax() = consumer<char>();
    } else if (op == opcode::CLEAN_ALL_DEFERS) {
      static CleanAllDeferBody body;
      pc() = body.value;
    } else if (op == opcode::CLEAN_FN_DEFERS) {
      static CleanFnDeferBody body;
      pc() = body.value;
    } else if (op == opcode::COMPARE) {
      ax() = pop() <=> ax();
    } else if (op == opcode::DIV) {
      staticast<double>(peek()) /= ax();
    } else if (op == opcode::END) {
      itCoroutine->alive = false;
      yield();
      return;
    } else if (op == opcode::EQ) {
      ax() = pop() == ax();
    } else if (op == opcode::FORWARD) {
      size_t argc = consume<size_t>();
      copy(sp() - argc - 1, sp(), bp() - 1);
      sp() = bp() + argc;
    } else if (op == opcode::GET_CALLEE) {
      ax() = callee();
    } else if (op == opcode::GET_CLOSURE) {
      size_t i = consume<size_t>();
      ax() = callee()->closureDefs[i];
    } else if (op == opcode::GET_GLOBAL) {
      auto name = consume<const string *>();
      ax() = globalDefs[name];
    } else if (op == opcode::GET_HIGH_REF) {
      ax() = staticast<ValueObj *>(ax())->value;
    } else if (op == opcode::GET_LOCAL) {
      size_t i = consume<size_t>();
      ax() = bp()[i];
    } else if (op == opcode::GET_MEMB) {
      ax() = getMemb(pop(), ax());
    } else if (op == opcode::GLOBAL_FORWARD) {
      size_t argc = consume<size_t>();
      copy(sp() - argc - 1, sp(), itCoroutine->valuek);
      sp() = itCoroutine->valuek + argc + 1;
    } else if (op == opcode::GT) {
      ax() = pop() > ax();
    } else if (op == opcode::GTEQ) {
      ax() = pop() >= ax();
    } else if (op == opcode::JIF) {
      size_t n = consume<size_t>();
      if (ax()) {
        pc() += n;
      }
    } else if (op == opcode::JMP) {
      pc() += consume<size_t>();
    } else if (op == opcode::LENGTH) {
      ax() = length(ax());
    } else if (op == opcode::LOGIC_NOT) {
      ax() = !ax();
    } else if (op == opcode::LOGIC_XOR) {
      peek() = (bool) peek() ^ (bool) ax();
    } else if (op == opcode::LSH) {
      staticast<double>(peek()) = staticast<int>(peek()) << (int) ax();
    } else if (op == opcode::LT) {
      ax() = pop() < ax();
    } else if (op == opcode::LTEQ) {
      ax() = pop() <= ax();
    } else if (op == opcode::MAKE_FN) {
      size_t paramn = consume<size_t>();
      size_t closureDefn = consume<size_t>();
      size_t bodyn = consume<size_t>();
      ax() = gc::neobj(new(closureDefn) FunctionObj(paramn, pc()));
      pc() += bodyn;
    } else if (op == opcode::MOD) {
      staticast<double>(peek()) = fmod(staticast<double>(peek()), (double) ax());
    } else if (op == opcode::MUL) {
      staticast<double>(peek()) *= (double) ax();
    } else if (op == opcode::NEGATIVE) {
      ax() = -(double) ax();
    } else if (op == opcode::NULL_LITERAL) {
      ax() = Null();
    } else if (op == opcode::NUM_LITERAL) {
      ax() = consume<double>();
    } else if (op == opcode::POP) {
      ax() = pop();
    } else if (op == opcode::POP_BP) {
      bp() = popOther<void *>();
    } else if (op == opcode::POP_PC) {
      pc() = popOther<void *>();
    } else if (op == opcode::POP_SP) {
      sp() = popOther<void *>();
    } else if (op == opcode::POSITIVE) {
      ax() = (double) ax();
    } else if (op == opcode::POW) {
      staticast<double>(peek()) = pow(staticast<double>(peek()), (double) ax());
    } else if (op == opcode::PUSH) {
      push(ax());
    } else if (op == opcode::PUSH_BP) {
      pushOther(bp());
    } else if (op == opcode::PUSH_DEFER) {
      pushDefer(ax());
    } else if (op == opcode::PUSH_PC_JMP) {
      size_t n = consume<size_t>();
      pushOther(pc() + n);
    } else if (op == opcode::PUSH_SP_BACK) {
      size_t n = consume<size_t>();
      pushOther(sp() - n);
    } else if (op == opcode::PUSH_TRY) {
      pushDefer(katch);
    } else if (op == opcode::RSH) {
      staticast<double>(peek()) = staticast<int>(peek()) >> (int) ax();
    } else if (op == opcode::SET_FN_CLOSURE) {
      auto f = staticast<FunctionObj *>(peek());
      size_t i = consume<size_t>();
      f->closureDefs[i] = ax();
    } else if (op == opcode::SET_GLOBAL) {
      globalDefs[consume<const string *>()] = ax();
    } else if (op == opcode::SET_HIGH_REF) {
      staticast<ValueObj *>(pop())->value = ax();
    } else if (op == opcode::SET_LOCAL) {
      size_t i = consume<size_t>();
      bp()[i] = ax();
    } else if (op == opcode::SET_MEMB) {
      auto key = pop();
      auto cont = pop();
      setMemb(cont, key, ax());
    } else if (op == opcode::STRING_LITERAL) {
      ax() = consume<const string *>();
    } else if (op == opcode::SUB) {
      staticast<double>(peek()) -= (double) ax();
    } else if (op == opcode::THROW) {
      // TODO:
    } else if (op == opcode::USH) {
      staticast<double>(peek()) = staticast<unsigned>(peek()) >> (int) ax();
    } else if (op == opcode::WRAP_HIGH_REF) {
      size_t i = consume<size_t>();
      auto vo = gc::neobj(new ValueObj);
      vo->value = bp()[i];
      bp()[i] = vo;
    } else if (op == opcode::YIELD) {
      yield();
      return;
    }
    exec();
  }

  template<class T>
  static T &konsume(void *&p) noexcept {
    auto &t = *(T *) p;
    p += sizeof(T);
    return t;
  }

  CleanAllDeferBody::CleanAllDeferBody() {
    void *p = value;
    konsume<char>(p) = opcode::ANYMORE_DEFER;
    konsume<char>(p) = opcode::POP_DEFER;
    konsume<char>(p) = opcode::PUSH_TRY;
    konsume<char>(p) = opcode::CALL;
    konsume<size_t>(p) = 0;
    konsume<char>(p) = opcode::JMP_TO;
    konsume<void *>(p) = value;
  }

  CleanFnDeferBody::CleanFnDeferBody() {
    void *p = value;
    konsume<char>(p) = opcode::ANYMORE_FN_DEFER;
    konsume<char>(p) = opcode::POP_DEFER;
    konsume<char>(p) = opcode::PUSH_TRY;
    konsume<char>(p) = opcode::CALL;
    konsume<size_t>(p) = 0;
    konsume<char>(p) = opcode::JMP_TO;
    konsume<void *>(p) = value;
  }
}
