#pragma once

#include"value.hh"

namespace zlt::mylisp {
  NativeFunction natfn_strcat;
  NativeFunction natfn_strjoin;
  NativeFunction natfn_strslice;
  NativeFunction natfn_strtod;
  NativeFunction natfn_strtoi;
  NativeFunction natfn_strtolower;
  NativeFunction natfn_strtoupper;
  NativeFunction natfn_strtrim;
  NativeFunction natfn_strtriml;
  NativeFunction natfn_strtrimr;
  NativeFunction natfn_strview;
}
