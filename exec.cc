#include<algorithm>
#include<cmath>
#include"mylisp.hh"
#include"object.hh"
#include"opcode.hh"
#include"value.hh"

namespace zlt::mylisp {
  static void call(size_t argc);
  static void cleanAllDefers();
  static void yield();

  using namespace it_coroutine;

  void exec() {
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
      // TODO:
    } else if (op == opcode::CLEAN_FN_DEFERS) {
      // TODO:
    } else if (op == opcode::COMPARE) {
      ax() = pop() <=> ax();
    } else if (op == opcode::DIV) {
      staticast<double>(peek()) /= ax();
    } else if (op == opcode::EQ) {
      ax() = pop() == ax();
    } else if (op == opcode::FORWARD) {
      size_t argc = consume<size_t>();
      copy(sp() - argc - 1, sp(), bp() - 1);
      sp() = bp() + argc;
      call(argc);
      return;
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
      // TODO:
    } else if (op == opcode::GLOBAL_FORWARD) {
      size_t argc = consume<size_t>();
      copy(sp() - argc - 1, sp(), itCoroutine->valuek);
      sp() = itCoroutine->valuek + argc + 1;
      call(argc);
      return;
    } else if (op == opcode::GLOBAL_RETURN) {
      // TODO:
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
      // TODO:
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
      // TODO:
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
    } else if (op == opcode::POSITIVE) {
      ax() = (double) ax();
    } else if (op == opcode::POW) {
      staticast<double>(peek()) = pow(staticast<double>(peek()), (double) ax());
    } else if (op == opcode::PUSH) {
      // TODO:
    } else if (op == opcode::PUSH_DEFER) {
      // TODO:
    } else if (op == opcode::PUSH_TRY) {
      // TODO:
    } else if (op == opcode::RETURN) {
      // TODO:
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
      // TODO:
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
      auto vo = neobj<ValueObj>();
      vo->value = bp()[i];
      bp()[i] = vo;
    } else if (op == opcode::YIELD) {
      yield();
      return;
    }
    exec();
  }

  void cleanAllDefers() {
    static const auto s = make_tuple(
      (char) opcode::JIF_ANYMORE_DEFER,
      (size_t) 0,
      (char) opcode::POP_DEFER,
      (char) opcode::PUSH_TRY,
      (char) opcode::CALL,
      (size_t) 0,
      (char) opcode::NULL_LITERAL,
      (char) opcode::THROW,);
    pushValue(ax());
    pushOther(pc());
  }
}
