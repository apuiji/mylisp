#pragma once

#include"object.hh"

namespace zlt::mylisp {
  template<class C>
  struct BasicStringViewObj: virtual Object {
    // cast operations begin
    virtual operator std::basic_string_view<C>() const noexcept = 0;
    bool objDynamicast(std::basic_string_view<C> &dest) const noexcept override {
      dest = operator std::basic_string_view<C>();
      return true;
    }
    // cast operations end
    // comparisons begin
    bool operator ==(const Value &v) const noexcept override {
      std::basic_string_view<C> sv;
      return dynamicast(sv, v) && operator std::basic_string_view<C>() == sv;
    }
    bool compare(int &dest, const Value &v) const noexcept override {
      std::basic_string_view<C> sv;
      if (dynamicast(sv, v)) {
        dest = operator std::basic_string_view<C>().compare(sv);
        return true;
      } else {
        return false;
      }
    }
    bool operator ==(std::basic_string_view<C> sv) const noexcept override {
      return operator std::basic_string_view<C>() == sv;
    }
    bool compare(int &dest, std::basic_string_view<C> sv) const noexcept override {
      dest = operator std::basic_string_view<C>().compare(sv);
      return true;
    }
    // comparisons end
    // member operations begin
    Value objGetMemb(const Value &memb) const noexcept override;
    // member operations end
  };

  template<class C>
  struct BasicStringObj final: BasicStringViewObj<C> {
    std::basic_string<C> value;
    BasicStringObj(const std::basic_string<C> &value) noexcept: value(value) {}
    BasicStringObj(std::basic_string<C> &&value) noexcept: value(std::move(value)) {}
    operator std::basic_string_view<C>() const noexcept override {
      return (std::basic_string_view<C>) value;
    }
  };

  using StringObj = BasicStringObj<wchar_t>;
  using Latin1Obj = BasicStringObj<char>;

  template<class C>
  struct BasicStringViewObj1 final: BasicStringViewObj<C> {
    Value string;
    std::basic_string_view<C> view;
    BasicStringViewObj1(const Value &string, std::basic_string_view<C> view) noexcept: string(string), view(view) {}
    int graySubjs() noexcept override {
      gc::grayValue(string);
      return 0;
    }
    operator std::basic_string_view<C>() const noexcept override {
      return view;
    }
  };

  using StringViewObj = BasicStringViewObj1<wchar_t>;
  using Latin1ViewObj = BasicStringViewObj1<char>;

  template<class C>
  Value BasicStringViewObj<C>::objGetMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    auto sv = operator std::basic_string_view<C>();
    if (i >= 0 && i < sv.size()) {
      return sv[i];
    } else {
      return Null();
    }
  }

  NativeFunction natfn_charcode;
  NativeFunction natfn_charcodes;
  NativeFunction natfn_chars;
  NativeFunction natfn_fromcharcode;
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
