#pragma once

#include<iostream>
#include"object.hh"

namespace zlt::mylisp::io {
  struct InputStreamViewObj: virtual Object {
    InputStreamViewObj() noexcept = default;
    virtual std::istream &getInputStream() noexcept = 0;
  };

  struct OutputStreamViewObj: virtual Object {
    OutputStreamViewObj() noexcept = default;
    virtual std::ostream &getOutputStream() noexcept = 0;
  };

  template<std::derived_from<std::istream> T>
  struct InputStreamObj final: InputStreamViewObj {
    T value;
    InputStreamObj(T &&value) noexcept: value(std::move(value)) {}
    std::istream &getInputStream() noexcept override {
      return value;
    }
  };

  template<std::derived_from<std::ostream> T>
  struct OutputStreamObj final: OutputStreamViewObj {
    T value;
    OutputStreamObj(T &&value) noexcept: value(std::move(value)) {}
    std::ostream &getOutputStream() noexcept override {
      return value;
    }
  };

  struct StdinObj final: InputStreamViewObj {
    StdinObj() noexcept = default;
    std::istream &getInputStream() noexcept override {
      return std::cin;
    }
  };

  struct StdoutObj final: OutputStreamViewObj {
    std::ostream &value;
    StdoutObj(std::ostream &value) noexcept: value(value) {}
    std::ostream &getOutputStream() noexcept override {
      return value;
    }
  };

  NativeFunction input;
  NativeFunction output;
}
