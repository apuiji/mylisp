#include"io_obj.hh"

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
}
