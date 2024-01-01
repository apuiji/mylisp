#include<sstream>
#include"compile.hh"
#include"direction.hh"

using namespace std;

namespace zlt::mylisp {
  struct Compile {
    ostream &ostr;
    Compile(ostream &ostr) noexcept: ostr(ostr) {}
  };

  template<class T>
  static inline Compile &operator <<(Compile &dest, T &&t) {
    dest.ostr << t;
    return dest;
  }

  static Compile &operator <<(Compile &dest, const UNode &src);

  int compile(string &dest, const UNode &src) {
    stringstream ss;
    rtol(Compile(ss)) << src;
    dest = ss.str();
    return 0;
  }

  #define declCompile(T) \
  static Compile &operator <<(Compile &dest, const T &src)

  template<class T>
  declCompile(LiteralAtom<T>);
  // ast_trans.hh definitions begin
  declCompile(Call);
  declCompile(Callee);
  declCompile(Defer);
  declCompile(Forward);
  declCompile(If);
  declCompile(Null);
  declCompile(Return);
  declCompile(Throw);
  declCompile(Try);
  declCompile(Yield);
  // operations begin
  declCompile(AssignOper);
  declCompile(Operation<1>);
  template<int N>
  declCompile(Operation<N>);
  // operations end
  // ast_trans.hh definitions end
  // ast_trans1.hh definitions begin
  declCompile(Argument);
  declCompile(CleanArguments);
  declCompile(Reference1);
  // ast_trans1.hh definitions end
  // ast_trans2.hh definitions begin
  declCompile(Function2);
  declCompile(GetPointerOper);
  declCompile(InputClosure);
  declCompile(MakePointer);
  declCompile(SetPointerOper);
  // ast_trans2.hh definitions end

  #undef declCompile

  Compile &operator <<(Compile &dest, const UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<const T *>(src.get()); a) { \
      return dest << *a; \
    }
    ifType(NumberAtom);
    ifType(CharAtom);
    ifType(StringAtom);
    ifType(Latin1Atom);
    // ast_trans.hh definitions begin
    ifType(Call);
    ifType(Callee);
    ifType(Defer);
    ifType(Forward);
    ifType(If);
    ifType(Null);
    ifType(Return);
    ifType(Throw);
    ifType(Try);
    ifType(Yield);
    ifType(AssignOper);
    ifType(Operation<1>);
    ifType(Operation<2>);
    ifType(Operation<3>);
    ifType(Operation<-1>);
    // ast_trans.hh definitions end
    // ast_trans1.hh definitions begin
    ifType(Argument);
    ifType(CleanArguments);
    ifType(Reference1);
    // ast_trans1.hh definitions end
    // ast_trans2.hh definitions begin
    ifType(Function2);
    ifType(GetPointerOper);
    ifType(InputClosure);
    ifType(MakePointer);
    ifType(SetPointerOper);
    // ast_trans2.hh definitions end
    #undef ifType
    // never
    return o;
  }

  template<class T>
  static consteval uint8_t setALiteralDir() {
    if constexpr (is_same_v<T, double>) {
      return direction::SET_A_NUM;
    } else if constexpr (is_same_v<T, wchar_t>) {
      return direction::SET_A_CHAR;
    } else if constexpr (is_same_v<T, const wstring *>) {
      return direction::SET_A_STR;
    } else if constexpr (is_same_v<T, const string *>) {
      return direction::SET_A_LATIN1;
    } else {
      // never
      return 0;
    }
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

  template<class T>
  Compile &operator <<(Compile &dest, const LiteralAtom<T> &src) {
    return dest << setALiteralDir<T>() << src.value;
  }

  template<class It>
  static Compile &compile(Compile &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return dest;
    }
    dest << *it << direction::PUSH;
    return compile(dest, ++it, end);
  }

  static inline Compile &operator <<(Compile &dest, const UNodes &src) {
    return compile(dest, src.begin(), src.end());
  }

  Compile &operator <<(Compile &dest, const Call &src) {
    return dest << src.callee << direction::PUSH << src.args << direction::CALL << src.args.size();
  }

  Compile &operator <<(Compile &dest, const Callee &src) {
    return dest << direction::SET_A_CALLEE;
  }

  Compile &operator <<(Compile &dest, const Defer &src) {
    return dest << src.item << direction::PUSH_DEFER;
  }

  Compile &operator <<(Compile &dest, const Forward &src) {
    return dest << src.callee << direction::PUSH << src.args << direction::FORWARD << src.args.size();
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

  Compile &operator <<(Compile &dest, const Return &src) {
    return dest << src.value << direction::RETURN;
  }

  Compile &operator <<(Compile &dest, const Throw &src) {
    return dest << src.value << direction::THROW;
  }

  Compile &operator <<(Compile &dest, const Try &src) {
    string body;
    compile(body, src.body);
    return dest << direction::TRY << body.size() << body;
  }

  Compile &operator <<(Compile &dest, const Yield &src) {
    return dest << direction::YIELD << src.then;
  }


}
