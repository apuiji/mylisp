#include<algorithm>
#include<cmath>
#include<cwctype>
#include"strings.hh"

using namespace std;

namespace zlt::mylisp {
  static int strcat1(wstringstream &dest, const Value *it, const Value *end);

  Value natFnStrcat(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return L"";
    }
    wstringstream ss;
    strcat1(ss, it, end);
    return ss.str();
  }

  int strcat1(wstringstream &dest, const Value *it, const Value *end) {
    write(dest, *it);
    return it + 1 != end ? strcat1(dest, it + 1, end) : 0;
  }

  static int strjoin1(wstringstream &dest, wstring_view sepa, ListObj::ConstIterator it, ListObj::ConstIterator end);

  Value natFnStrjoin(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return L"";
    }
    ListObj *lso;
    if (!dynamicast(lso, *it)) {
      return toStringValue(*it);
    }
    wstring_view sepa;
    if (!(it + 1 != end && dynamicast(sepa, it[1]))) {
      sepa = L"";
    }
    wstringstream ss;
    strjoin1(ss, sepa, lso->list.begin(), lso->list.end());
    return ss.str();
  }

  int strjoin1(wstringstream &dest, wstring_view sepa, ListObj::ConstIterator it, ListObj::ConstIterator end) {
    write(dest, *it);
    if (it + 1 == end) {
      return 0;
    }
    dest << sepa;
    return strjoin1(dest, sepa, it + 1, end);
  }

  static int strview1(wchar_t &dest, const wstring *&dest1, wstring_view &dest2, const Value *it, const Value *end);

  Value natFnStrslice(const Value *it, const Value *end) {
    wchar_t c;
    const wstring *s;
    wstring_view sv;
    switch (strview1(c, s, sv, it, end)) {
      case 0: {
        return c;
      }
      case 1: {
        return s;
      }
      case 2: {
        return wstring(sv);
      }
      default: {
        return Null();
      }
    }
  }

  Value natFnStrview(const Value *it, const Value *end) {
    wchar_t c;
    const wstring *s;
    wstring_view sv;
    switch (strview1(c, s, sv, it, end)) {
      case 0: {
        return c;
      }
      case 1: {
        return s;
      }
      case 2: {
        return gc::neobj(new StringViewObj(*it, sv));
      }
      default: {
        return Null();
      }
    }
  }

  int strview1(wchar_t &dest, const wstring *&dest1, wstring_view &dest2, const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return -1;
    }
    wstring_view sv;
    if (!dynamicast(sv, *it)) {
      return -1;
    }
    int sstart = it + 1 != end ? max((int) it[1], 0) : 0;
    int send = sv.size();
    send = it + 2 < end ? min((int) it[2], send) : send;
    if (!(sstart < send)) {
      dest1 = L"";
      return 1;
    }
    size_t n = send - sstart;
    switch (n) {
      case 0: {
        dest1 = L"";
        return 1;
      }
      case 1: {
        dest = sv[sstart];
        return 0;
      }
      default: {
        dest2 = sv.substr(sstart, n);
        return 2;
      }
    }
  }

  Value natFnStrtod(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return NAN;
    }
    wstring_view sv;
    if (!dynamicast(sv, *it)) {
      return NAN;
    }
    try {
      return stod(sv);
    } catch (...) {
      return NAN;
    }
  }

  Value natFnStrtoi(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    wstring_view sv;
    if (!dynamicast(sv, *it)) {
      return Null();
    }
    int base;
    if (!(it + 1 != end && dynamicast(base, it[1]))) {
      base = 10;
    }
    try {
      return stoi(sv, nullptr, base);
    } catch (...) {
      return Null();
    }
  }

  Value natFnStrtolower(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    wstring_view sv;
    if (!dynamicast(sv, *it)) {
      return Null();
    }
    struct It: wstring_view::iterator {
      using iterator::iterator;
      wchar_t operator *() noexcept {
        return tolower(iterator::operator *());
      }
    };
    return wstring(It(sv.begin()), It(sv.end()));
  }

  Value natFnStrtoupper(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    wstring_view sv;
    if (!dynamicast(sv, *it)) {
      return Null();
    }
    struct It: wstring_view::iterator {
      using iterator::iterator;
      wchar_t operator *() noexcept {
        return toupper(iterator::operator *());
      }
    };
    return wstring(It(sv.begin()), It(sv.end()));
  }
}
