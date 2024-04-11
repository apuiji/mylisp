#include<algorithm>
#include<cmath>
#include"mylisp.hh"
#include"object.hh"
#include"opcode.hh"
#include"value.hh"

namespace zlt::mylisp {
  static void call(size_t argc);
  static void yield();

  template<class T>
  static inline T consume() noexcept {
    T t = *(T *) itCoroutine->pc;
    itCoroutine->pc += sizeof(T);
    return t;
  }

  void exec() {
    int op = *itCoroutine->pc++;
    if (op == opcode::ADD) {
      itCoroutine->ax = staticast<double>(*--itCoroutine->sp) + (double) itCoroutine->ax;
    } else if (op == opcode::BIT_AND) {
      itCoroutine->ax = staticast<int>(*--itCoroutine->sp) & (int) itCoroutine->ax;
    } else if (op == opcode::BIT_NOT) {
      itCoroutine->ax = ~(int) itCoroutine->ax;
    } else if (op == opcode::BIT_OR) {
      itCoroutine->ax = staticast<int>(*--itCoroutine->sp) | (int) itCoroutine->ax;
    } else if (op == opcode::BIT_XOR) {
      itCoroutine->ax = staticast<int>(*--itCoroutine->sp) ^ (int) itCoroutine->ax;
    } else if (op == opcode::CALL) {
      size_t argc = consume<size_t>();
      call(argc);
      return;
    } else if (op == opcode::CHAR_LITERAL) {
      itCoroutine->ax = *itCoroutine->pc++;
    } else if (op == opcode::CLEAN_ALL_DEFERS) {
      // TODO:
    } else if (op == opcode::CLEAN_ARGS) {
      itCoroutine->sp = itCoroutine->bp;
    } else if (op == opcode::CLEAN_FN_DEFERS) {
      // TODO:
    } else if (op == opcode::COMPARE) {
      itCoroutine->ax = *--itCoroutine->sp <=> itCoroutine->ax;
    } else if (op == opcode::DIV) {
      itCoroutine->ax = staticast<double>(*--itCoroutine->sp) / itCoroutine->ax;
    } else if (op == opcode::EQ) {
      itCoroutine->ax = *--itCoroutine->sp == itCoroutine->ax;
    } else if (op == opcode::FORWARD) {
      size_t argc = consume<size_t>();
      copy(itCoroutine->sp - argc - 1, itCoroutine->sp, itCoroutine->bp - 1);
      itCoroutine->sp = itCoroutine->bp + argc;
      call(argc);
      return;
    } else if (op == opcode::GET_ARG) {
      size_t i = consume<size_t>();
      itCoroutine->ax = itCoroutine->bp[i];
    } else if (op == opcode::GET_CALLEE) {
      itCoroutine->ax = staticast<FunctionRTObj *>(itCoroutine->bp[-1])->callee;
    } else if (op == opcode::GET_CLOSURE) {
      auto f = staticast<FunctionRTObj *>(itCoroutine->bp[-1])->callee;
      auto name = consume<const string *>();
      itCoroutine->ax = f->closureDefs[name];
    } else if (op == opcode::GET_GLOBAL) {
      auto name = consume<const string *>();
      itCoroutine->ax = globalDefs[name];
    } else if (op == opcode::GET_HIGH_REF) {
      itCoroutine->ax = staticast<ValueObj *>(itCoroutine->ax)->value;
    } else if (op == opcode::GET_LOCAL) {
      auto f = staticast<FunctionRTObj *>(itCoroutine->bp[-1]);
      auto name = consume<const string *>();
      itCoroutine->ax = f->localDefs[name];
    } else if (op == opcode::GET_MEMB) {
      // TODO:
    } else if (op == opcode::GLOBAL_FORWARD) {
      size_t argc = consume<size_t>();
      copy(itCoroutine->sp - argc - 1, itCoroutine->sp, itCoroutine->valuek.data);
      itCoroutine->sp = itCoroutine->valuek.data + argc + 1;
      call(argc);
      return;
    } else if (op == opcode::GLOBAL_RETURN) {
      // TODO:
    } else if (op == opcode::GT) {
      itCoroutine->ax = *--itCoroutine->sp > itCoroutine->ax;
    } else if (op == opcode::GTEQ) {
      itCoroutine->ax = *--itCoroutine->sp >= itCoroutine->ax;
    } else if (op == opcode::JIF) {
      size_t n = consume<size_t>();
      if (itCoroutine->ax) {
        itCoroutine->pc += n;
      }
    } else if (op == opcode::JMP) {
      itCoroutine->pc += consume<size_t>();
    } else if (op == opcode::LENGTH) {
      // TODO:
    } else if (op == opcode::LOGIC_NOT) {
      itCoroutine->ax = !itCoroutine->ax;
    } else if (op == opcode::LOGIC_XOR) {
      itCoroutine->ax = (bool) *--itCoroutine->sp ^ (bool) itCoroutine->ax;
    } else if (op == opcode::LSH) {
      itCoroutine->ax = staticast<int>(*--itCoroutine->sp) << (int) itCoroutine->ax;
    } else if (op == opcode::LT) {
      itCoroutine->ax = *--itCoroutine->sp < itCoroutine->ax;
    } else if (op == opcode::LTEQ) {
      itCoroutine->ax = *--itCoroutine->sp <= itCoroutine->ax;
    } else if (op == opcode::MAKE_FN) {
      size_t paramn = consume<size_t>();
      auto body = consumer<const string *>();
      itCoroutine->ax = neobj<FunctionObj>(paramn, body);
    } else if (op == opcode::MAKE_HIGH_REF) {
      itCoroutine->ax = neobj<ValueObj>();
    } else if (op == opcode::MOD) {
      itCoroutine->ax = fmod(staticast<double>(*--itCoroutine->sp), (double) itCoroutine->ax);
    } else if (op == opcode::MUL) {
      itCoroutine->ax = staticast<double>(*--itCoroutine->sp) * (double) itCoroutine->ax;
    } else if (op == opcode::NEGATIVE) {
      itCoroutine->ax = -(double) itCoroutine->ax;
    } else if (op == opcode::NULL_LITERAL) {
      itCoroutine->ax = Null();
    } else if (op == opcode::NUM_LITERAL) {
      itCoroutine->ax = consume<double>();
    } else if (op == opcode::POP) {
      itCoroutine->ax = *--itCoroutine->sp;
    } else if (op == opcode::POSITIVE) {
      itCoroutine->ax = (double) itCoroutine->ax;
    } else if (op == opcode::POW) {
      itCoroutine->ax = pow(staticast<double>(*--itCoroutine->sp), (double) itCoroutine->ax);
    } else if (op == opcode::PUSH) {
      // TODO:
    } else if (op == opcode::PUSH_DEFER) {
      // TODO:
    } else if (op == opcode::PUSH_TRY) {
      // TODO:
    } else if (op == opcode::RETURN) {
      // TODO:
    } else if (op == opcode::RSH) {
      itCoroutine->ax = staticast<int>(*--itCoroutine->sp) >> (int) itCoroutine->ax;
    } else if (op == opcode::SET_FN_CLOSURE) {
      auto f = staticast<FunctionObj *>(itCoroutine->sp[-1]);
      f->closureDefs[consume<const string *>()] = itCoroutine->ax;
    } else if (op == opcode::SET_GLOBAL) {
      globalDefs[consume<const string *>()] = itCoroutine->ax;
    } else if (op == opcode::SET_HIGH_REF) {
      staticast<ValueObj *>(*--itCoroutine->sp)->value = itCoroutine->ax;
    } else if (op == opcode::SET_LOCAL) {
      auto f = staticast<FunctionRTObj *>(itCoroutine->bp[-1]);
      auto name = consume<const string *>();
      f->localDefs[name] = itCoroutine->ax;
    } else if (op == opcode::SET_MEMB) {
      // TODO:
    } else if (op == opcode::STRING_LITERAL) {
      itCoroutine->ax = consume<const string *>();
    } else if (op == opcode::SUB) {
      itCoroutine->ax = staticast<double>(*--itCoroutine->sp) - (double) itCoroutine->ax;
    } else if (op == opcode::THROW) {
      // TODO:
    } else if (op == opcode::USH) {
      itCoroutine->ax = staticast<unsigned>(*--itCoroutine->sp) - (int) itCoroutine->ax;
    } else if (op == opcode::YIELD) {
      yield();
      return;
    }
    exec();
  }
}
