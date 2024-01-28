#include<ostream>
#include"object.hh"

namespace zlt::mylisp {
  template<class C>
  struct BasicOutputObj final: Object {
    std::basic_ostream<C> &ostream;
    BasicOutputObj(std::basic_ostream<C> &ostream) noexcept: ostream(ostream) {}
  };

  using OutputObj = BasicOutputObj<char>;
  using WriterObj = BasicOutputObj<wchar_t>;

  int output(std::ostream &dest, const Value &src);
  int write(std::wostream &dest, const Value &src);

  Value natFnOutput(const Value *it, const Value *end);
  Value natFnWrite(const Value *it, const Value *end);
}