#pragma once

#include<string>

namespace zlt::mylisp::ast {
  namespace token {
    enum {
      NUMBER = 1,
      CHAR,
      STRING,
      ID,
      E0F,
      X
    };

    consteval int symbol(std::string_view s, auto ...s1) {
      int i = 0;
      ((++i, s == s1) || ... || (i = 0));
      return i;
    }

    template<size_t N>
    struct Symbol {
      int value;
      consteval Symbol(const char (&s)[N]):
      value(
        symbol(
          s,
          "callee",
          "def",
          "defer",
          "forward",
          "if",
          "length",
          "return",
          "throw",
          "try",
          "yield",
          "!",
          "#",
          "##",
          "#def",
          "#ifdef",
          "#ifndef",
          "#include",
          "#undef",
          "%",
          "&&",
          "&",
          "(",
          ")",
          "**",
          "*",
          "+",
          ",",
          "-",
          ".",
          "/",
          "<<",
          "<=>",
          "<=",
          "<",
          "==",
          "=",
          ">=",
          ">>>",
          ">>",
          ">",
          "@",
          "^^",
          "^",
          "||",
          "|",
          "~")) {}
    };

    int ofRaw(double &numval, const char *start, std::string_view raw);
  }

  template<token::Symbol symb>
  consteval int operator "" _token() {
    static_assert(symb.value);
    return symb.value;
  }
}
