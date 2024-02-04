#include"io.hh"

using namespace std;

namespace zlt::mylisp {
  Value natfn_getc(const Value *it, const Value *end) {
    InputObj *io;
    if (!dynamicast(io, it, end)) [[unlikely]] {
      return Null();
    }
    wchar_t c = io->istream.get();
    return c;
  }

  template<class To, class From>
  static inline bool output1(basic_ostream<To> &dest, const Value *it, const Value *end) {
    basic_string_view<From> src;
    if (!dynamicast(src, it, end)) [[unlikely]] {
      return false;
    }
    if constexpr (is_same_v<To, From>) {
      dest << src;
    } else {
      iconv_t ic;
      if (it + 1 == end || !it[1]) {
        ic = defaultIconv<To, From>;
      } else if (IconvObj1<To, From> *ico; dynamicast(ico, it + 1, end)) {
        ic = ico->value;
      } else {
        return false;
      }
      output(dest, ic, src);
    }
    return true;
  }

  int output(ostream &dest, const Value *it, const Value *end) {
    output1<char, char>(dest, it, end) || output1<char, wchar_t>(dest, it, end);
    return 0;
  }

  int output(wostream &dest, const Value *it, const Value *end) {
    output1<wchar_t, wchar_t>(dest, it, end) || output1<wchar_t, char>(dest, it, end);
    return 0;
  }

  template<class To>
  static inline int output2(const Value *it, const Value *end) {
    BasicOutputObj<To> *oo;
    if (dynamicast(oo, it, end)) {
      output(oo->ostream, it + 1, end);
    }
    return 0;
  }

  Value natfn_output(const Value *it, const Value *end) {
    output2<char>(it, end);
    return Null();
  }

  Value natfn_write(const Value *it, const Value *end) {
    output2<wchar_t>(it, end);
    return Null();
  }
}
