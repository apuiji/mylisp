#include"io.hh"

using namespace std;

namespace zlt::mylisp {
  int output(ostream &dest, const Value &src) {
    if (string_view s; dynamicast(s, src)) {
      dest << s;
    }
    return 0;
  }

  int write(wostream &dest, const Value &src) {
    switch (src.index()) {
      case Value::NUM_INDEX: {
        double d;
        staticast(d, src);
        dest << d;
        return 0;
      }
      default: {
        wstring_view s;
        if (dynamicast(s, src)) {
          dest << s;
        }
        return 0;
      }
    }
  }

  Value natFnWrite(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    WriterObj *wo;
    if (!dynamicast(wo, *it)) [[unlikely]] {
      return Null();
    }
    if (it + 1 == end) [[unlikely]] {
      return Null();
    }
    write(wo->ostream, it[1]);
    return Null();
  }

  Value natFnOutput(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    OutputObj *oo;
    if (!dynamicast(oo, *it)) [[unlikely]] {
      return Null();
    }
    if (it + 1 == end) [[unlikely]] {
      return Null();
    }
    output(oo->ostream, it[1]);
    return Null();
  }
}
