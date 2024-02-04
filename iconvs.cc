#include<cerrno>
#include"iconvs.hh"
#include"strings.hh"

using namespace std;

namespace zlt::mylisp {
  template<>
  iconv_t defaultIconv<char, wchar_t>;
  template<>
  iconv_t defaultIconv<wchar_t, char>;

  Value natfn_iconv(const Value *it, const Value *end) {
    IconvObj *ic;
    string_view src;
    if (!dynamicasts(make_tuple(&ic, &src), it, end)) [[unlikely]] {
      return Null();
    }
    string s;
    if (strconv(s, ic->value, src)) {
      return neobj<Latin1Obj>(std::move(s));
    } else {
      return Null();
    }
  }

  Value natfn_iconv_close(const Value *it, const Value *end) {
    IconvObj *a;
    if (dynamicast(a, it, end)) {
      iconv_close(a->value);
      a->value = (iconv_t) -1;
    }
    return 0;
  }

  enum {
    CHAR_CHAR, CHAR_WCHAR, WCHAR_CHAR
  };

  static bool open(iconv_t &dest, const Value *it, const Value *end);

  Value natfn_iconv_open(const Value *it, const Value *end) {
    iconv_t ic;
    if (open(ic, it, end)) {
      return neobj<IconvObj>(ic);
    } else {
      return Null();
    }
  }

  static int charsetNames(wstring_view &to, wstring_view &from, const Value *it, const Value *end) noexcept;

  static inline bool open1(iconv_t &dest, const char *to, const char *from) noexcept {
    dest = iconv_open(to, from);
    return dest != (iconv_t) -1;
  }

  bool open(iconv_t &dest, const Value *it, const Value *end) {
    wstring_view to;
    wstring_view from;
    switch (charsetNames(to, from, it, end)) {
      case CHAR_CHAR: {
        string to1;
        string from1;
        return strconv(to1, to) && strconv(from1, from) && open1(dest, to1.data(), from1.data());
      }
      case CHAR_WCHAR: {
        string to1;
        return strconv(to1, to) && open1(dest, to1.data(), "WCHAR_T");
      }
      case WCHAR_CHAR: {
        string from1;
        return strconv(from1, from) && open1(dest, "WCHAR_T", from1.data());
      }
      default: {
        return false;
      }
    }
  }

  int charsetNames(wstring_view &to, wstring_view &from, const Value *it, const Value *end) noexcept {
    if (it == end) [[unlikely]] {
      return -1;
    }
    if (*it) {
      if (!dynamicast(to, *it)) [[unlikely]] {
        return -1;
      }
      if (it + 1 == end || !it[1]) {
        return CHAR_WCHAR;
      }
      if (!dynamicast(from, it[1])) [[unlikely]] {
        return -1;
      }
      return CHAR_CHAR;
    } else {
      if (it + 1 == end || !it[1]) {
        return -1;
      }
      if (!dynamicast(from, it[1])) [[unlikely]] {
        return -1;
      }
      return WCHAR_CHAR;
    }
  }

  template<class To, class From>
  static inline Value xxcode(const Value *it, const Value *end) {
    basic_string_view<From> src;
    if (!dynamicast(src, it, end)) [[unlikely]] {
      return Null();
    }
    iconv_t ic;
    if (it + 1 == end || !it[1]) {
      ic = defaultIconv<To, From>;
    } else if (IconvObj1<To, From> *a; dynamicast(a, it[1])) {
      ic = a->value;
    } else {
      return Null();
    }
    basic_string<To> s;
    if (strconv(s, ic, src)) {
      return neobj<BasicStringObj<To>>(std::move(s));
    } else {
      return Null();
    }
  }

  Value natfn_strdec(const Value *it, const Value *end) {
    return xxcode<char, wchar_t>(it, end);
  }

  Value natfn_strenc(const Value *it, const Value *end) {
    return xxcode<wchar_t, char>(it, end);
  }

  static inline size_t strconv2(char *dest, size_t n, iconv_t ic, const char *src, size_t m) noexcept {
    size_t n1 = n;
    iconv(ic, (char **) &src, &m, &dest, &n1);
    return n - n1;
  }

  template<class To>
  static inline bool strconv1(basic_string<To> &dest, size_t n, iconv_t ic, const char *src, size_t m) {
    dest.resize(n);
    errno = 0;
    size_t n1 = strconv2((char *) dest.data(), n * sizeof(To), ic, src, m);
    switch (errno) {
      case 0: {
        dest.resize(n1 / sizeof(To));
        dest.shrink_to_fit();
        return true;
      }
      case E2BIG: {
        return strconv1(dest, n << 1, ic, src, m);
      }
      default: {
        return false;
      }
    }
  }

  bool strconv(string &dest, iconv_t ic, string_view src) {
    return strconv1(dest, src.size(), ic, src.data(), src.size());
  }

  bool strconv(string &dest, iconv_t ic, wstring_view src) {
    size_t n = src.size() * sizeof(wchar_t);
    return strconv1(dest, n, ic, (const char *) src.data(), n);
  }

  bool strconv(wstring &dest, iconv_t ic, string_view src) {
    return strconv1(dest, src.size() / sizeof(wchar_t), ic, src.data(), src.size());
  }
}
