#pragma once

#include<cstdint>
#include<utility>

namespace zlt::mylisp::ast::token {
  enum: uint64_t {
    NUMBER = 1,
    CHAR,
    STRING,
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
    PPD_def,
    PPD_file,
    PPD_idcat,
    PPD_ifdef,
    PPD_ifndef,
    PPD_include,
    PPD_line,
    PPD_toString,
    PPD_undef,
    // preproc directions end
    E0F
  };

  template<size_t N, size_t ...I>
  requires (N <= 8)
  static inline constexpr uint64_t symbol(const char (&s)[N], std::index_sequence<I...>) noexcept {
    uint64_t u = 0;
    ((u = (u << 8) | s[I]), ...);
    return 0x80'00'00'00'00'00'00'00UL | u;
  }

  template<size_t N>
  static inline constexpr uint64_t symbol(const char (&s)[N]) noexcept {
    return symbol(s, std::make_index_sequence<N - 1>());
  }
}
