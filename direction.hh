#pragma once

namespace zlt::mylisp::direction {
  enum: uint8_t {
    NOP = 0,
    ADD,
    BIT_AND,
    BIT_NOT,
    BIT_OR,
    BIT_XOR,
    CALL,
    CLN_ARGS,
    CMP,
    DIV,
    EQ,
    FORWARD,
    GET_ARG,
    GET_CLOSURE,
    GET_GLOBAL,
    GET_LOCAL,
    GET_MEMB,
    GET_PTR,
    GT,
    GTEQ,
    INPUT_CLOSURE,
    JIF,
    JMP,
    LSH,
    LT,
    LTEQ,
    MAKE_FN,
    MAKE_PTR,
    MOD,
    MUL,
    NOT,
    POP,
    POP_DEFER,
    POP_FORWARD,
    POW,
    PUSH,
    PUSH_DEFER,
    RETURN,
    RSH,
    SET_CALLEE,
    SET_CHAR,
    SET_GLOBAL,
    SET_LATIN1,
    SET_LOCAL,
    SET_MEMB,
    SET_NULL,
    SET_NUM,
    SET_PTR,
    SET_STR,
    SUB,
    THROW,
    TRY,
    USH,
    XOR,
    YIELD
  };
}
