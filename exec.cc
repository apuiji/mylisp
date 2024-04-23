#include<algorithm>
#include<cmath>
#include"gc.hh"
#include"mylisp.hh"
#include"object.hh"
#include"opcode.hh"
#include"vm.hh"

using namespace std;

namespace zlt::mylisp {
  struct CleanFnGuardBody {
    char value[128];
    CleanFnGuardBody();
  };

  struct CleanGuardBody {
    char value[128];
    CleanGuardBody();
  };

  static void call(size_t argc);
  static NativeFunction katch;

  void exec() {
    using namespace vm;
    int op = *pc;
    ++pc;
    if (op == opcode::ADD) {
      staticast<double>(valuek::peek()) += (double) ax;
    } else if (op == opcode::BIT_AND) {
      staticast<double>(valuek::peek()) = staticast<int>(valuek::peek()) & (int) ax;
    } else if (op == opcode::BIT_NOT) {
      ax = ~(int) ax;
    } else if (op == opcode::BIT_OR) {
      staticast<double>(valuek::peek()) = staticast<int>(valuek::peek()) | (int) ax;
    } else if (op == opcode::BIT_XOR) {
      staticast<double>(valuek::peek()) = staticast<int>(valuek::peek()) ^ (int) ax;
    } else if (op == opcode::CALL) {
      size_t argc = consume<size_t>();
      call(argc);
      return;
    } else if (op == opcode::CATCH_NAT_FN) {
      ax = katch;
    } else if (op == opcode::CHAR_LITERAL) {
      ax = consume<char>();
    } else if (op == opcode::CLEAN_FN_GUARDS) {
      static CleanFnGuardBody body;
      pc = body.value;
    } else if (op == opcode::CLEAN_GUARDS) {
      static CleanGuardBody body;
      pc = body.value;
    } else if (op == opcode::COMPARE) {
      ax = valuek::pop() <=> ax;
    } else if (op == opcode::DIV) {
      staticast<double>(valuek::peek()) /= (double) ax;
    } else if (op == opcode::EQ) {
      ax = valuek::pop() == ax;
    } else if (op == opcode::FORWARD) {
      size_t argc = consume<size_t>();
      copy(sp - argc - 1, sp, bp - 1);
      sp = bp + argc;
    } else if (op == opcode::GET_CALLEE) {
      ax = callee();
    } else if (op == opcode::GET_CLOSURE) {
      size_t i = consume<size_t>();
      ax = callee()->closureDefs[i];
    } else if (op == opcode::GET_GLOBAL) {
      auto name = consume<const string *>();
      auto a = mymap::find(globalDefs, name);
      if (a) {
        ax = a->value.second;
      } else {
        ax = Null();
      }
    } else if (op == opcode::GET_HIGH_REF) {
      ax = staticast<ValueObj *>(ax)->value;
    } else if (op == opcode::GET_LOCAL) {
      size_t i = consume<size_t>();
      ax = bp[i];
    } else if (op == opcode::GET_MEMB) {
      ax = getMemb(valuek::pop(), ax);
    } else if (op == opcode::GT) {
      ax = valuek::pop() > ax;
    } else if (op == opcode::GTEQ) {
      ax = valuek::pop() >= ax;
    } else if (op == opcode::INC_FN_GUARD) {
      ++callee()->guardn;
    } else if (op == opcode::JIF) {
      size_t n = consume<size_t>();
      if (ax) {
        pc += n;
      }
    } else if (op == opcode::JMP) {
      pc += consume<size_t>();
    } else if (op == opcode::LENGTH) {
      if (size_t n; length(n, ax)) {
        ax = n;
      } else {
        ax = Null();
      }
    } else if (op == opcode::LOGIC_NOT) {
      ax = !ax;
    } else if (op == opcode::LOGIC_XOR) {
      valuek::peek() = (bool) valuek::peek() ^ (bool) ax;
    } else if (op == opcode::LSH) {
      staticast<double>(valuek::peek()) = staticast<int>(valuek::peek()) << (int) ax;
    } else if (op == opcode::LT) {
      ax = valuek::pop() < ax;
    } else if (op == opcode::LTEQ) {
      ax = valuek::pop() <= ax;
    } else if (op == opcode::MAKE_FN) {
      size_t paramn = consume<size_t>();
      size_t closureDefn = consume<size_t>();
      size_t bodyn = consume<size_t>();
      auto f = new(closureDefn) FunctionObj(paramn, pc);
      gc::neobj(f);
      ax = f;
      pc += bodyn;
    } else if (op == opcode::MOD) {
      staticast<double>(valuek::peek()) = fmod(staticast<double>(valuek::peek()), (double) ax);
    } else if (op == opcode::MORE_FN_GUARD) {
      ax = callee()->guardn > 0;
    } else if (op == opcode::MUL) {
      staticast<double>(valuek::peek()) *= (double) ax;
    } else if (op == opcode::NEGATIVE) {
      ax = -(double) ax;
    } else if (op == opcode::NULL_LITERAL) {
      ax = Null();
    } else if (op == opcode::NUM_LITERAL) {
      ax = consume<double>();
    } else if (op == opcode::POP) {
      ax = valuek::pop();
    } else if (op == opcode::POP_BP) {
      bp = otherk::pop<Value *>();
    } else if (op == opcode::POP_DEFER) {
      ax = deferk::pop();
    } else if (op == opcode::POP_GUARD) {
      ax = guardk::pop();
    } else if (op == opcode::POP_PC) {
      pc = otherk::pop<char *>();
    } else if (op == opcode::POP_SP) {
      sp = bp;
    } else if (op == opcode::POSITIVE) {
      ax = (double) ax;
    } else if (op == opcode::POW) {
      staticast<double>(valuek::peek()) = pow(staticast<double>(valuek::peek()), (double) ax);
    } else if (op == opcode::PUSH) {
      valuek::push(ax);
    } else if (op == opcode::PUSH_BP) {
      otherk::push(bp);
    } else if (op == opcode::PUSH_DEFER) {
      deferk::push(ax);
    } else if (op == opcode::PUSH_GUARD) {
      guardk::push(ax);
    } else if (op == opcode::PUSH_PC_JMP) {
      size_t n = consume<size_t>();
      otherk::push(pc + n);
    } else if (op == opcode::PUSH_SP_BACK) {
      size_t n = consume<size_t>();
      bp = sp - n;
    } else if (op == opcode::RSH) {
      staticast<double>(valuek::peek()) = staticast<int>(valuek::peek()) >> (int) ax;
    } else if (op == opcode::SET_FN_CLOSURE) {
      auto f = staticast<FunctionObj *>(valuek::peek());
      size_t i = consume<size_t>();
      f->closureDefs[i] = ax;
    } else if (op == opcode::SET_GLOBAL) {
      auto key = consume<const string *>();
      mymap::Node<const std::string *, Value> *a;
      if (mymap::insert(a, globalDefs, key, [] () { return new mymap::Node<const std::string *, Value>(); })) {
        a->value.first = key;
      }
      a->value.second = ax;
    } else if (op == opcode::SET_HIGH_REF) {
      staticast<ValueObj *>(valuek::pop())->value = ax;
    } else if (op == opcode::SET_LOCAL) {
      size_t i = consume<size_t>();
      bp[i] = ax;
    } else if (op == opcode::SET_MEMB) {
      auto key = valuek::pop();
      auto cont = valuek::pop();
      setMemb(cont, key, ax);
    } else if (op == opcode::STRING_LITERAL) {
      ax = consume<const string *>();
    } else if (op == opcode::SUB) {
      staticast<double>(valuek::peek()) -= (double) ax;
    } else if (op == opcode::USH) {
      staticast<double>(valuek::peek()) = staticast<unsigned>(valuek::peek()) >> (int) ax;
    } else if (op == opcode::WRAP_HIGH_REF) {
      size_t i = consume<size_t>();
      auto vo = new ValueObj;
      gc::neobj(vo);
      vo->value = bp[i];
      bp[i] = vo;
    }
    exec();
  }

  template<class T>
  static T &konsume(char *&p) noexcept {
    auto &t = *(T *) p;
    p += sizeof(T);
    return t;
  }

  CleanFnGuardBody::CleanFnGuardBody() {
    // (
    //   if more_fn_guard
    //     (try (pop guardk)) ; A
    //     (forward callee) ; B
    //   if 1
    //     (return) ; C
    // )
    char *p = value;
    konsume<char>(p) = opcode::MORE_FN_GUARD;
    konsume<char>(p) = opcode::JIF;
    konsume<size_t>(p) = 1;
    // C begin
    konsume<char>(p) = opcode::POP_PC;
    // C end
    // A begin
    konsume<char>(p) = opcode::POP_GUARD;
    konsume<char>(p) = opcode::PUSH;
    konsume<char>(p) = opcode::PUSH_BP;
    konsume<char>(p) = opcode::PUSH_SP_BACK;
    konsume<size_t>(p) = 1;
    konsume<char>(p) = opcode::CATCH_NAT_FN;
    konsume<char>(p) = opcode::PUSH_GUARD;
    konsume<char>(p) = opcode::PUSH_PC_JMP;
    konsume<size_t>(p) = 4 + sizeof(size_t);
    konsume<char>(p) = opcode::CALL;
    konsume<size_t>(p) = 0;
    konsume<char>(p) = opcode::NULL_LITERAL;
    konsume<char>(p) = opcode::PUSH;
    konsume<char>(p) = opcode::CLEAN_GUARDS;
    konsume<char>(p) = opcode::POP_SP;
    konsume<char>(p) = opcode::POP_BP;
    // A end
    // C begin
    konsume<char>(p) = opcode::CLEAN_FN_GUARDS;
    // C end
  }

  CleanGuardBody::CleanGuardBody() {
    // (try (pop guardk)) ; A
    // (forward callee) ; B
    char *p = value;
    // A begin
    konsume<char>(p) = opcode::POP_GUARD;
    konsume<char>(p) = opcode::PUSH;
    konsume<char>(p) = opcode::PUSH_BP;
    konsume<char>(p) = opcode::PUSH_SP_BACK;
    konsume<size_t>(p) = 1;
    konsume<char>(p) = opcode::CATCH_NAT_FN;
    konsume<char>(p) = opcode::PUSH_GUARD;
    konsume<char>(p) = opcode::PUSH_PC_JMP;
    konsume<size_t>(p) = 4 + sizeof(size_t);
    konsume<char>(p) = opcode::CALL;
    konsume<size_t>(p) = 0;
    konsume<char>(p) = opcode::NULL_LITERAL;
    konsume<char>(p) = opcode::PUSH;
    konsume<char>(p) = opcode::CLEAN_GUARDS;
    konsume<char>(p) = opcode::POP_SP;
    konsume<char>(p) = opcode::POP_BP;
    // A end
    // B begin
    konsume<char>(p) = opcode::CLEAN_GUARDS;
    // B end
  }

  static void call(FunctionObj &fo, Value *args, size_t argc);

  void call(size_t argc) {
    using namespace vm;
    auto args = sp - argc;
    if (FunctionObj *fo; dynamicast(fo, args[-1])) {
      call(*fo, args, argc);
      return;
    }
    if (NativeFunction *nf; dynamicast(nf, args[-1])) {
      nf(args, argc);
      pc = otherk::pop<char *>();
      exec();
      return;
    }
    ax = Null();
    pc = otherk::pop<char *>();
    exec();
  }

  void call(FunctionObj &fo, Value *args, size_t argc) {
    using namespace vm;
    if (fo.paramn < argc) {
      sp -= argc - fo.paramn;
    } else if (fo.paramn > argc) {
      size_t n = fo.paramn - argc;
      auto end = sp + n;
      if (end > valuek::end) {
        // TODO: out of stack bad
      }
      for (; sp != end; ++sp) {
        *sp = Null();
      }
    }
    pc = fo.body;
    exec();
  }

  void katch(const Value *args, size_t argc) {
    using namespace vm;
    ax = valuek::pop();
    // discard 1 pc
    otherk::pop<char *>();
  }
}
