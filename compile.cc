#include<sstream>
#include"ast_nodes3.hh"
#include"compile.hh"
#include"direction.hh"
#include"rte.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp {
  using namespace ast;

  struct Compile {
    ostream &ostr;
    Compile(ostream &ostr) noexcept: ostr(ostr) {}
  };

  static inline Compile &operator <<(Compile &dest, const string &s) {
    dest.ostr << s;
    return dest;
  }

  static Compile &operator <<(Compile &dest, const UNode &src);

  template<class It>
  struct Sequence {
    It begin;
    It end;
    Sequence(It begin, It end) noexcept: begin(begin), end(end) {}
  };

  template<class It>
  static inline auto makeSequence(It begin, It end) noexcept {
    return Sequence<It>(begin, end);
  }

  template<class R>
  static inline auto makeSequence(R &&r) noexcept {
    return makeSequence(r.begin(), r.end());
  }

  template<class It>
  static inline Compile &operator <<(Compile &dest, const Sequence<It> &src) {
    for (It it = src.begin; it != src.end; ++it) {
      dest << *it;
    }
    return dest;
  }

  int compile(string &dest, UNodes::const_iterator it, UNodes::const_iterator end) {
    stringstream ss;
    Compile comp(ss);
    comp << makeSequence(it, end);
    dest = ss.str();
    return 0;
  }

  #define declCompile(T) \
  static Compile &operator <<(Compile &dest, const T &src)

  declCompile(CharAtom);
  declCompile(StringAtom);
  // ast_trans.hh definitions begin
  declCompile(Call);
  declCompile(Callee);
  declCompile(Defer);
  declCompile(Forward);
  declCompile(If);
  declCompile(ast::Null);
  declCompile(Number);
  declCompile(Return);
  declCompile(Throw);
  declCompile(Try);
  declCompile(Yield);
  // operations begin
  #define COMMA ,
  template<int Op>
  declCompile(Operation1<1 COMMA Op>);
  template<int Op>
  declCompile(Operation1<2 COMMA Op>);
  template<int Op>
  declCompile(Operation1<3 COMMA Op>);
  template<int Op>
  declCompile(Operation1<-1 COMMA Op>);
  #undef COMMA
  declCompile(AssignOper);
  declCompile(LogicAndOper);
  declCompile(LogicOrOper);
  declCompile(SetMemberOper);
  // operations end
  // ast_trans.hh definitions end
  // ast_trans1.hh definitions begin
  declCompile(Argument);
  declCompile(CleanArguments);
  declCompile(Reference1);
  // ast_trans1.hh definitions end
  // ast_trans2.hh definitions begin
  declCompile(Function2);
  declCompile(GetIndirectOper);
  declCompile(InputClosure);
  declCompile(MakeIndirect);
  declCompile(SequenceOper);
  declCompile(SetIndirectOper);
  // ast_trans2.hh definitions end

  #undef declCompile

  Compile &operator <<(Compile &dest, const UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<const T *>(src.get()); a) { \
      return dest << *a; \
    }
    ifType(CharAtom);
    ifType(StringAtom);
    // ast_trans.hh definitions begin
    ifType(Call);
    ifType(Callee);
    ifType(Defer);
    ifType(Forward);
    ifType(If);
    ifType(ast::Null);
    ifType(Number);
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
    ifType(SequenceOper);
    ifType(SetMemberOper);
    // operations end
    // ast_trans.hh definitions end
    // ast_trans1.hh definitions begin
    ifType(Argument);
    ifType(CleanArguments);
    ifType(Reference1);
    // ast_trans1.hh definitions end
    // ast_trans2.hh definitions begin
    ifType(Function2);
    ifType(GetIndirectOper);
    ifType(InputClosure);
    ifType(MakeIndirect);
    ifType(SetIndirectOper);
    // ast_trans2.hh definitions end
    #undef ifType
    // never
    return dest;
  }

  template<class T>
  requires (is_scalar_v<T>)
  Compile &operator <<(Compile &dest, T t) {
    if constexpr (sizeof(T) == 1) {
      dest.ostr.put(t);
    } else {
      dest.ostr.write((const char *) &t, sizeof(T));
    }
    return dest;
  }

  Compile &operator <<(Compile &dest, const CharAtom &src) {
    return dest << direction::SET_CHAR << src.value;
  }

  Compile &operator <<(Compile &dest, const StringAtom &src) {
    return dest << direction::SET_STR << src.value;
  }

  template<class It>
  struct Args {
    It begin;
    It end;
    Args(It begin, It end) noexcept: begin(begin), end(end) {}
  };

  template<class It>
  static inline auto makeArgs(It begin, It end) noexcept {
    return Args<It>(begin, end);
  }

  template<class R>
  static inline auto makeArgs(const R &r) noexcept {
    return makeArgs(r.begin(), r.end());
  }

  template<class It>
  static inline Compile &operator <<(Compile &dest, const Args<It> &src) {
    for (It it = src.begin; it != src.end; ++it) {
      dest << *it << direction::PUSH;
    }
    return dest;
  }

  Compile &operator <<(Compile &dest, const Call &src) {
    return dest << src.callee << direction::PUSH << makeArgs(src.args) << direction::CALL << src.args.size();
  }

  Compile &operator <<(Compile &dest, const Callee &src) {
    return dest << direction::SET_CALLEE;
  }

  Compile &operator <<(Compile &dest, const Defer &src) {
    return dest << src.item << direction::PUSH_DEFER;
  }

  Compile &operator <<(Compile &dest, const Forward &src) {
    return dest << src.callee << direction::PUSH << makeArgs(src.args) << direction::FORWARD << src.args.size();
  }

  static inline int compile(string &dest, const UNode &src) {
    stringstream ss;
    Compile comp(ss);
    comp << src;
    dest = ss.str();
    return 0;
  }

  Compile &operator <<(Compile &dest, const If &src) {
    dest << src.cond;
    string then;
    compile(then, src.then);
    string elze;
    compile(elze, src.elze);
    dest << direction::JIF << (elze.size() + 1 + sizeof(size_t)) << elze;
    dest << direction::JMP << then.size() << then;
    return dest;
  }

  Compile &operator <<(Compile &dest, const ast::Null &src) {
    return dest << direction::SET_NULL;
  }

  Compile &operator <<(Compile &dest, const Number &src) {
    return dest << direction::SET_NUM << src.value;
  }

  Compile &operator <<(Compile &dest, const Return &src) {
    return dest << src.value << direction::RETURN;
  }

  Compile &operator <<(Compile &dest, const Throw &src) {
    return dest << src.value << direction::THROW;
  }

  Compile &operator <<(Compile &dest, const Try &src) {
    string body;
    compile(body, src.body.begin(), src.body.end());
    return dest << direction::TRY << body.size() << body;
  }

  Compile &operator <<(Compile &dest, const Yield &src) {
    return dest << direction::YIELD << src.then;
  }

  template<int Op>
  static consteval uint8_t operat0r() {
    if constexpr (Op == "!"_token) {
      return direction::NOT;
    } else if constexpr (Op == "%"_token) {
      return direction::MOD;
    } else if constexpr (Op == "&"_token) {
      return direction::BIT_AND;
    } else if constexpr (Op == "**"_token) {
      return direction::POW;
    } else if constexpr (Op == "*"_token) {
      return direction::MUL;
    } else if constexpr (Op == "+"_token) {
      return direction::ADD;
    } else if constexpr (Op == "-"_token) {
      return direction::SUB;
    } else if constexpr (Op == "."_token) {
      return direction::GET_MEMB;
    } else if constexpr (Op == "/"_token) {
      return direction::DIV;
    } else if constexpr (Op == "<<"_token) {
      return direction::LSH;
    } else if constexpr (Op == "<=>"_token) {
      return direction::CMP;
    } else if constexpr (Op == "<="_token) {
      return direction::LTEQ;
    } else if constexpr (Op == "<"_token) {
      return direction::LT;
    } else if constexpr (Op == "=="_token) {
      return direction::EQ;
    } else if constexpr (Op == ">="_token) {
      return direction::GTEQ;
    } else if constexpr (Op == ">>>"_token) {
      return direction::USH;
    } else if constexpr (Op == ">>"_token) {
      return direction::RSH;
    } else if constexpr (Op == ">"_token) {
      return direction::GT;
    } else if constexpr (Op == "^^"_token) {
      return direction::XOR;
    } else if constexpr (Op == "^"_token) {
      return direction::BIT_XOR;
    } else if constexpr (Op == "|"_token) {
      return direction::BIT_OR;
    } else if constexpr (Op == "~"_token) {
      return direction::BIT_NOT;
    } else {
      // never
      return 0;
    }
  }

  template<int Op>
  Compile &operator <<(Compile &dest, const Operation1<1, Op> &src) {
    return dest << src.item << operat0r<Op>();
  }

  template<int Op>
  Compile &operator <<(Compile &dest, const Operation1<2, Op> &src) {
    return dest << src.items[0] << direction::PUSH << src.items[1] << operat0r<Op>();
  }

  template<int Op>
  Compile &operator <<(Compile &dest, const Operation1<3, Op> &src) {
    return dest << src.items[0] << direction::PUSH << src.items[1] << direction::PUSH << src.items[2] << operat0r<Op>();
  }

  template<int Op>
  Compile &operator <<(Compile &dest, const Operation1<-1, Op> &src) {
    return dest << makeArgs(src.items) << operat0r<Op>() << src.items.size();
  }

  Compile &operator <<(Compile &dest, const AssignOper &src) {
    dest << src.items[1];
    auto &ref = static_cast<const Reference1 &>(*src.items[0]);
    if (ref.scope == Reference::LOCAL_SCOPE) {
      dest << direction::SET_LOCAL;
    } else {
      dest << direction::SET_GLOBAL;
    }
    return dest << ref.name;
  }

  static Compile &logicAnd(Compile &dest, UNodes::const_iterator it, UNodes::const_iterator end) {
    if (it == end) [[unlikely]] {
      return dest;
    }
    dest << *it;
    string then;
    {
      stringstream ss;
      Compile comp(ss);
      logicAnd(comp, ++it, end);
      then = ss.str();
    }
    if (then.size()) {
      dest << direction::JIF << (1 + sizeof(size_t)) << direction::JMP << then.size() << then;
    }
    return dest;
  }

  Compile &operator <<(Compile &dest, const LogicAndOper &src) {
    return logicAnd(dest, src.items.begin(), src.items.end());
  }

  static Compile &logicOr(Compile &dest, UNodes::const_iterator it, UNodes::const_iterator end) {
    if (it == end) [[unlikely]] {
      return dest;
    }
    dest << *it;
    string elze;
    {
      stringstream ss;
      Compile comp(ss);
      logicOr(comp, ++it, end);
      elze = ss.str();
    }
    if (elze.size()) {
      dest << direction::JIF << elze.size() << elze;
    }
    return dest;
  }

  Compile &operator <<(Compile &dest, const LogicOrOper &src) {
    return logicOr(dest, src.items.begin(), src.items.end());
  }

  Compile &operator <<(Compile &dest, const SetMemberOper &src) {
    return dest << makeArgs(src.items) << direction::SET_MEMB;
  }

  Compile &operator <<(Compile &dest, const Argument &src) {
    return dest << direction::GET_ARG << src.index;
  }

  Compile &operator <<(Compile &dest, const CleanArguments &src) {
    return dest << direction::CLN_ARGS;
  }

  Compile &operator <<(Compile &dest, const Reference &src) {
    if (src.scope == Reference::LOCAL_SCOPE) {
      dest << direction::GET_LOCAL;
    } else if (src.scope == Reference::CLOSURE_SCOPE) {
      dest << direction::GET_CLOSURE;
    } else {
      dest << direction::GET_GLOBAL;
    }
    return dest << src.name;
  }

  Compile &operator <<(Compile &dest, const Reference1 &src) {
    return dest << static_cast<const Reference &>(src);
  }

  Compile &operator <<(Compile &dest, const Function2 &src) {
    string body;
    compile(body, src.body.begin(), src.body.end());
    auto itBody = rte::fnBodies.insert(std::move(body)).first;
    dest << direction::MAKE_FN << &*itBody;
    if (src.inputClosures.size()) {
      dest << direction::PUSH << makeSequence(src.inputClosures) << direction::POP;
    }
    return dest;
  }

  Compile &operator <<(Compile &dest, const GetIndirectOper &src) {
    return dest << src.item << direction::GET_INDIRECT;
  }

  Compile &operator <<(Compile &dest, const InputClosure &src) {
    return dest << src.ref << direction::INPUT_CLOSURE << src.name;
  }

  Compile &operator <<(Compile &dest, const MakeIndirect &src) {
    return dest << direction::MAKE_INDIRECT;
  }

  Compile &operator <<(Compile &dest, const SequenceOper &src) {
    for (auto &a : src.items) {
      dest << a;
    }
    return dest;
  }

  Compile &operator <<(Compile &dest, const SetIndirectOper &src) {
    return dest << src.items[0] << direction::PUSH << src.items[1] << direction::SET_INDIRECT;
  }
}
