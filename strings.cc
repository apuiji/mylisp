#include<algorithm>
#include<cctype>
#include<cmath>
#include<sstream>
#include"io.hh"
#include"lists.hh"
#include"myccutils/constr.hh"
#include"myccutils/xyz.hh"
#include"strings.hh"

using namespace std;

namespace zlt::mylisp {
  Value CastStringViewObj::objGetMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    auto sv = view();
    if (i >= 0 && i < sv.size()) {
      return sv[i];
    } else {
      return Null();
    }
  }

  Value natfn_charcode(const Value *it, const Value *end) {
    string_view s;
    if (!dynamicast(s, it, end) || !s.size()) [[unlikely]] {
      return Null();
    }
    return (int) s[0];
  }

  Value natfn_charcodes(const Value *it, const Value *end) {
    string_view s;
    if (!dynamicast(s, it, end)) [[unlikely]] {
      return Null();
    }
    deque<Value> a(s.size());
    copy(s.begin(), s.end(), a.begin());
    return neobj<ListObj>(std::move(a));
  }

  Value natfn_chars(const Value *it, const Value *end) {
    string_view s;
    if (!dynamicast(s, it, end)) [[unlikely]] {
      return Null();
    }
    deque<Value> a(s.size());
    transform(s.begin(), s.end(), a.begin(), [] (wchar_t c) { return (int) c; });
    return neobj<ListObj>(std::move(a));
  }

  static int fromcharcode1(stringstream &dest, const Value *it, const Value *end);

  Value natfn_fromcharcode(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return constString<>;
    }
    stringstream ss;
    fromcharcode1(ss, it, end);
    return ss.str();
  }

  int fromcharcode1(stringstream &dest, const Value *it, const Value *end) {
    if (int c; dynamicast(c, *it) && c > 0) {
      dest.put(c);
    }
    return it + 1 != end ? fromcharcode1(dest, it + 1, end) : 0;
  }

  static int strcat1(stringstream &dest, const Value *it, const Value *end);

  Value natfn_strcat(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return constString<>;
    }
    stringstream ss;
    strcat1(ss, it, end);
    return ss.str();
  }

  int strcat1(stringstream &dest, const Value *it, const Value *end) {
    string_view s;
    if (dynamicast(s, *it)) {
      dest << s;
    }
    return it + 1 != end ? strcat1(dest, it + 1, end) : 0;
  }

  static int strjoin1(stringstream &dest, string_view sepa, ListObj::ConstIterator it, ListObj::ConstIterator end);

  Value natfn_strjoin(const Value *it, const Value *end) {
    ListObj *lso;
    if (!dynamicast(lso, it, end)) [[unlikely]] {
      if (string_view sv; dynamicast(sv, *it)) {
        return *it;
      } else {
        return constString<>;
      }
    }
    string_view sepa;
    dynamicast(sepa, it + 1, end);
    stringstream ss;
    strjoin1(ss, sepa, lso->list.begin(), lso->list.end());
    return ss.str();
  }

  int strjoin1(stringstream &dest, string_view sepa, ListObj::ConstIterator it, ListObj::ConstIterator end) {
    string_view s;
    if (dynamicast(s, *it)) {
      dest << s;
    }
    if (it + 1 == end) {
      return 0;
    }
    dest << sepa;
    return strjoin1(dest, sepa, it + 1, end);
  }

  template<bool Slice>
  static inline Value sliceOrView(const Value *it, const Value *end) {
    string_view sv;
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
      return constString<>;
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
    string_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return NAN;
    }
    try {
      return stod(string(sv));
    } catch (...) {
      return NAN;
    }
  }

  Value natfn_strtoi(const Value *it, const Value *end) {
    string_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return Null();
    }
    int base;
    if (!dynamicast(base, it + 1, end)) {
      base = 10;
    }
    try {
      return stoi(string(sv), nullptr, base);
    } catch (...) {
      return Null();
    }
  }

  template<bool Lower>
  static inline Value strtocase(const Value *it, const Value *end) {
    string_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return Null();
    }
    string s(sv.size(), L'\0');
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
  static inline string_view trim1(string_view src) noexcept {
    int start;
    if constexpr (L) {
      start = find_if_not(src.begin(), src.end(), [] (char c) { return isspace(c); }) - src.begin();
    } else {
      start = 0;
    }
    int end;
    if constexpr (R) {
      end = src.size() - (find_if_not(src.rbegin(), src.rend(), [] (char c) { return isspace(c); }) - src.rbegin());
    } else {
      end = src.size();
    }
    return src.substr(start, end);
  }

  template<bool L, bool R>
  static inline Value trim(const Value *it, const Value *end) {
    string_view sv;
    if (dynamicast(sv, it, end)) {
      return neobj<StringViewObj>(*it, trim1<L, R>(sv));
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
