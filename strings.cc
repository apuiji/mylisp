#include<algorithm>
#include<cmath>
#include<cwctype>
#include<sstream>
#include"io.hh"
#include"myccutils/xyz.hh"
#include"strings.hh"

using namespace std;

namespace zlt::mylisp {
  static int strcat1(wstringstream &dest, const Value *it, const Value *end);

  Value natFnStrcat(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return constring<>;
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
    ListObj *lso;
    if (!dynamicast(lso, it, end)) [[unlikely]] {
      if (wstring_view sv; dynamicast(sv, *it)) {
        return *it;
      } else {
        return constring<>;
      }
    }
    wstring_view sepa;
    dynamicast(sepa, it + 1, end);
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

  template<bool Slice>
  static inline Value sliceOrView(const Value *it, const Value *end) {
    wstring_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return Null();
    }
    int start;
    if (!dynamicast(start, it + 1, end) || start < 0) {
      start = 0;
    }
    int end1;
    if (!dynamicast(end1, it + 2, end) || end1 > sv.size()) {
      end1 = sv.size();
    }
    if (start >= end1) {
      return constring<>;
    }
    if (start == 0 && end1 == sv.size()) {
      return *it;
    }
    if constexpr (Slice) {
      return sv.substr(start, end1 - start);
    } else {
      return Value(*it, sv.substr(start, end1 - start));
    }
  }

  Value natFnStrslice(const Value *it, const Value *end) {
    return sliceOrView<true>(it, end);
  }

  Value natFnStrview(const Value *it, const Value *end) {
    return sliceOrView<false>(it, end);
  }

  Value natFnStrtod(const Value *it, const Value *end) {
    wstring_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return NAN;
    }
    try {
      return stod(wstring(sv));
    } catch (...) {
      return NAN;
    }
  }

  Value natFnStrtoi(const Value *it, const Value *end) {
    wstring_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return Null();
    }
    int base;
    if (!dynamicast(base, it + 1, end)) {
      base = 10;
    }
    try {
      return stoi(wstring(sv), nullptr, base);
    } catch (...) {
      return Null();
    }
  }

  template<bool Lower>
  static inline Value strtocase(const Value *it, const Value *end) {
    wstring_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return Null();
    }
    wstring s(sv.size(), L'\0');
    if constexpr (Lower) {
      transform(sv.begin(), sv.end(), s.begin(), towlower);
    } else {
      transform(sv.begin(), sv.end(), s.begin(), towupper);
    }
    return std::move(s);
  }

  Value natFnStrtolower(const Value *it, const Value *end) {
    return strtocase<true>(it, end);
  }

  Value natFnStrtoupper(const Value *it, const Value *end) {
    return strtocase<false>(it, end);
  }
}
