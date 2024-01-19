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

  Value natfn_strcat(const Value *it, const Value *end) {
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

  Value natfn_strjoin(const Value *it, const Value *end) {
    ListObj *lso;
    if (!dynamicast(lso, it, end)) [[unlikely]] {
      if (wstring_view sv; dynamicast(sv, *it)) {
        return *it;
      } else {
        return L"";
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
      return L"";
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

  Value natfn_strslice(const Value *it, const Value *end) {
    return sliceOrView<true>(it, end);
  }

  Value natfn_strview(const Value *it, const Value *end) {
    return sliceOrView<false>(it, end);
  }

  Value natfn_strtod(const Value *it, const Value *end) {
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

  Value natfn_strtoi(const Value *it, const Value *end) {
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

  Value natfn_strtolower(const Value *it, const Value *end) {
    return strtocase<true>(it, end);
  }

  Value natfn_strtoupper(const Value *it, const Value *end) {
    return strtocase<false>(it, end);
  }

  template<bool L, bool R>
  static inline wstring_view trim1(wstring_view src) noexcept {
    int start;
    if constexpr (L) {
      start = find_if_not(src.begin(), src.end(), iswspace) - src.begin();
    } else {
      start = 0;
    }
    int end;
    if constexpr (R) {
      end = src.size() - (find_if_not(src.rbegin(), src.rend(), iswspace) - src.rbegin());
    } else {
      end = src.size();
    }
    return src.substr(start, end);
  }

  template<bool L, bool R>
  static inline Value trim(const Value *it, const Value *end) {
    wstring_view sv;
    if (dynamicast(sv, it, end)) {
      return gc::neobj(new StringViewObj(*it, trim1<L, R>(sv)));
    } else {
      return Null();
    }
  }

  Value natfn_strtrim(const Value *it, const Value *end) {
    return trim<true, true>(it, end);
  }

  Value natfn_strtriml(const Value *it, const Value *end) {
    return trim<true, false>(it, end);
  }

  Value natfn_strtrimr(const Value *it, const Value *end) {
    return trim<false, true>(it, end);
  }
}
