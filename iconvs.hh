#pragma once

#include<iconv.h>
#include"object.hh"

namespace zlt::mylisp {
  extern iconv_t utf8towcs;
  extern iconv_t wcstoutf8;

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

  NativeFunction natfn_iconv_open;
  NativeFunction natfn_iconv_close;
  NativeFunction natfn_iconv;
  NativeFunction natfn_strenc;
  NativeFunction natfn_strdec;
}
