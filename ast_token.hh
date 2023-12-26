#pragma once

namespace zlt::mylisp::ast::token {
  enum {
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

  template<>
}
