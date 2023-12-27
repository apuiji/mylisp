#pragma once

#include<utility>

namespace zlt::mylisp::ast::token {
  enum: uint64_t {
    NUMBER = 1,
    CHAR,
    STRING,
    WSTRING,
    ID,
    LPAREN,
    RPAREN,
    // keywords begin
    KWD_callee,
    KWD_def,
    KWD_defer,
    KWD_forward,
    KWD_if,
    KWD_length,
    KWD_return,
    KWD_throw,
    KWD_try,
    KWD_yield,
    // keywords end
    // preproc directions begin
    DIR_def,
    DIR_ifndef,
    DIR_include,
    DIR_undef,
    // preproc directions end
    E0F
  };

  template<size_t N, size_t ...I>
  requires (N <= 8)
  static inline constexpr uint64_t symbol(const char (&s)[N], std::index_sequence<I...>) noexcept {
    uint64_t u = 0;
    ((u = (u << 8) | s[I]), ...);
    return 0x8000000000000000UL | u;
  }

  template<size_t N>
  static inline constexpr uint64_t symbol(const char (&s)[N]) noexcept {
    return symbol(s, std::make_index_sequence<N - 1>());
  }
}
