#pragma once

#include<cstdint>
#include<utility>

namespace zlt::mylisp::ast {
  namespace token {
    enum: uint64_t {
      NUMBER = 1,
      CHAR,
      STRING,
      ID,
      LPAREN,
      RPAREN,
      E0F
    };

    template<size_t ...I>
    requires (sizeof...(I) <= 8)
    consteval uint64_t symbol(const char *s, std::index_sequence<I...>) {
      uint64_t u = 0;
      ((u = (u << 8) | s[I]), ...);
      return 0x80'00'00'00'00'00'00'00UL | u;
    }

    template<size_t N>
    struct Symbol {
      uint64_t value;
      consteval Symbol(const char (&s)[N]): value(symbol(s, std::make_index_sequence<N - 1>())) {}
    };
  }

  template<token::Symbol symb>
  consteval uint64_t operator "" _token() {
    return symb.value;
  }
}
