#include<istream>
#include<ostream>
#include"iconvs.hh"

namespace zlt::mylisp {
  template<class C>
  struct BasicInputObj final: Object {
    std::basic_istream<C> &istream;
    BasicInputObj(std::basic_istream<C> &istream) noexcept: istream(istream) {}
  };

  using InputObj = BasicInputObj<char>;
  using ReadObj = BasicInputObj<wchar_t>;

  template<class C>
  struct BasicOutputObj final: Object {
    std::basic_ostream<C> &ostream;
    BasicOutputObj(std::basic_ostream<C> &ostream) noexcept: ostream(ostream) {}
  };

  using OutputObj = BasicOutputObj<char>;
  using WriterObj = BasicOutputObj<wchar_t>;

  template<class To, class From>
  int output(std::basic_ostream<To> &dest, iconv_t ic, std::basic_string_view<From> src) {
    std::basic_string<To> s;
    if (strconv(s, ic, src)) {
      dest << s;
    }
    return 0;
  }

  int output(std::ostream &dest, const Value *it, const Value *end);
  int output(std::wostream &dest, const Value *it, const Value *end);

  // native functions begin
  Value natfn_output(const Value *it, const Value *end);
  Value natfn_write(const Value *it, const Value *end);
  // native functions end
}
