#pragma once

#include<iconv.h>
#include"object.hh"

namespace zlt::mylisp {
  struct IconvObj: Object {
    iconv_t value;
    IconvObj(iconv_t value) noexcept: value(value) {}
    ~IconvObj() noexcept {
      iconv_close(value);
    }
  };

  template<class To, class From>
  struct IconvObj1 final: IconvObj {
    using IconvObj::IconvObj;
  };

  template<class To, class From>
  iconv_t defaultIconv;
  template<>
  extern iconv_t defaultIconv<char, wchar_t>;
  template<>
  extern iconv_t defaultIconv<wchar_t, char>;

  NativeFunction natfn_iconv;
  NativeFunction natfn_iconv_close;
  NativeFunction natfn_iconv_open;
  NativeFunction natfn_strdec;
  NativeFunction natfn_strenc;

  bool strconv(std::string &dest, iconv_t ic, std::string_view src);
  bool strconv(std::string &dest, iconv_t ic, std::wstring_view src);
  bool strconv(std::wstring &dest, iconv_t ic, std::string_view src);

  static inline bool strconv(std::string &dest, std::wstring_view src) {
    return strconv(dest, defaultIconv<char, wchar_t>, src);
  }

  static inline bool strconv(std::wstring &dest, std::string_view src) {
    return strconv(dest, defaultIconv<wchar_t, char>, src);
  }
}
