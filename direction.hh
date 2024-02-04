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
    CONTINUE_FORWARD,
    CONTINUE_RETURN,
    CONTINUE_THROW,
    DIV,
    EQ,
    FORWARD,
    GET_ARG,
    GET_CLOSURE,
    GET_GLOBAL,
    GET_INDIRECT,
    GET_LOCAL,
    GET_MEMB,
    GT,
    GTEQ,
    INPUT_CLOSURE,
    JIF,
    JMP,
    LSH,
    LT,
    LTEQ,
    MAKE_FN,
    MAKE_INDIRECT,
    MOD,
    MUL,
    NOT,
    POP,
    POW,
    PUSH,
    PUSH_DEFER,
    RETURN,
    RSH,
    SET_CALLEE,
    SET_CHAR,
    SET_GLOBAL,
    SET_INDIRECT,
    SET_LATIN1,
    SET_LOCAL,
    SET_MEMB,
    SET_NULL,
    SET_NUM,
    SET_STR,
    SUB,
    THROW,
    TRY,
    USH,
    XOR,
    YIELD
  };
}
