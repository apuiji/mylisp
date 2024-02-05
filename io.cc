#include"io.hh"

using namespace std;

namespace zlt::mylisp {
  int output(ostream &dest, const Value *it, const Value *end) {
    string_view src;
    if (!dynamicast(src, it, end)) [[unlikely]] {
      return false;
    }
    dest << src;
    return it + 1 != end ? output(dest, it + 1, end) : 0;
  }

  Value natfn_getc(const Value *it, const Value *end) {
    InputObj *io;
    if (!dynamicast(io, it, end)) [[unlikely]] {
      return Null();
    }
    char c = io->value.get();
    return c;
  }

  Value natfn_output(const Value *it, const Value *end) {
    OutputObj *oo;
    if (dynamicast(oo, it, end)) {
      output(oo->value, it + 1, end);
    }
    return 0;
  }
}
