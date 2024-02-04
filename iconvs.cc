#include"iconvs.hh"

using namespace std;

namespace zlt::mylisp {
  enum {
    CHAR_CHAR, CHAR_WCHAR, WCHAR_CHAR
  };

  static int open1(wstring_view &to, wstring_view &from, const Value *it, const Value *end) noexcept;

  Value natfn_open(const Value *it, const Value *end) {
    wstring_view to;
    wstring_view from;
    switch (open1(to, from, it, end)) {
      case CHAR_CHAR:
    }
  }
}
