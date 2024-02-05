#pragma once

#include"object.hh"

namespace zlt::mylisp {
  struct CastStringViewObj: virtual Object {
    // cast operations begin
    virtual std::string_view view() const noexcept = 0;
    bool objDynamicast(std::string_view &dest) const noexcept override {
      dest = view();
      return true;
    }
    // cast operations end
    // comparisons begin
    bool operator ==(const Value &v) const noexcept override {
      std::string_view sv;
      return dynamicast(sv, v) && view() == sv;
    }
    bool compare(int &dest, const Value &v) const noexcept override {
      std::string_view sv;
      if (dynamicast(sv, v)) {
        dest = view().compare(sv);
        return true;
      } else {
        return false;
      }
    }
    bool operator ==(std::string_view sv) const noexcept override {
      return view() == sv;
    }
    bool compare(int &dest, std::string_view sv) const noexcept override {
      dest = view().compare(sv);
      return true;
    }
    // comparisons end
    // member operations begin
    Value objGetMemb(const Value &memb) const noexcept override;
    // member operations end
  };

  struct StringObj final: CastStringViewObj {
    std::string value;
    StringObj(const std::string &value) noexcept: value(value) {}
    StringObj(std::string &&value) noexcept: value(std::move(value)) {}
    std::string_view view() const noexcept override {
      return (std::string_view) value;
    }
  };

  struct StringViewObj final: CastStringViewObj {
    Value string;
    std::string_view value;
    StringViewObj(const Value &string, std::string_view value) noexcept: string(string), value(value) {}
    int graySubjs() noexcept override {
      gc::grayValue(string);
      return 0;
    }
    std::string_view view() const noexcept override {
      return value;
    }
  };

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
