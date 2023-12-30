#pragma once

#include<array>
#include<set>
#include"ast_include.hh"

namespace zlt::mylisp::ast {
  int trans(UNode &dest, UNode &src);

  struct TransBad {
    const Pos *pos;
    std::string what;
    TransBad(const Pos *pos, std::string &&what) noexcept: pos(pos), what(std::move(what)) {}
  };

  struct Call final: Node {
    UNode callee;
    UNodes args;
    Call(const Pos *pos, UNode &&callee, UNodes &&args) noexcept: Node(pos), callee(std::move(callee)), args(std::move(args)) {
    }
  };

  struct Callee final: Node {
    using Node::Node;
  };

  struct Defer final: Node {
    UNode item;
    Defer(const Pos *pos, UNode &&item) noexcept: Node(pos), item(std::move(item)) {}
  };

  struct Forward final: Node {
    UNode callee;
    UNodes args;
    Forward(const Pos *pos, UNode &&callee, UNodes &&args) noexcept:
    Node(pos), callee(std::move(callee)), args(std::move(args)) {}
  };

  struct Function final: Node {
    std::set<const std::wstring *> defs;
    std::vector<const std::wstring *> params;
    UNode body;
    Function(const Pos *pos, std::set<const std::wstring *> &&defs, std::vector<const std::wstring *> &&params, UNode &&body)
    noexcept: Node(pos), defs(std::move(defs)), params(std::move(params)), body(std::move(body)) {}
  };

  struct If final: Node {
    UNode cond;
    UNode then;
    UNode elze;
    If(const Pos *pos, UNode &&cond, UNode &&then, UNode &&elze) noexcept:
    Node(pos), cond(std::move(cond)), then(std::move(then)), elze(std::move(elze)) {}
  };

  struct Null final: Node {
    using Node::Node;
  };

  struct Return final: Node {
    UNode value;
    Return(const Pos *pos, UNode &&value) noexcept: Node(pos), value(std::move(value)) {}
  };

  struct Throw final: Node {
    UNode value;
    Throw(const Pos *pos, UNode &&value) noexcept: Node(pos), value(std::move(value)) {}
  };

  struct Try final: Node {
    UNode body;
    Try(const Pos *pos, UNode &&body) noexcept: Node(pos), body(std::move(body)) {}
  };

  struct Yield final: Node {
    UNode then;
    Yield(const Pos *pos, UNode &&then) noexcept: Node(pos), then(std::move(then)) {}
  };

  // operations begin
  template<int N>
  struct Operation: Node {
    std::array<UNode, N> items;
    Operation(const Pos *pos, std::array<UNode, N> &&items) noexcept: Node(pos), items(std::move(items)) {}
  };

  template<>
  struct Operation<1>: Node {
    UNode item;
    Operation(const Pos *pos, UNode &&item) noexcept: Node(pos), item(std::move(item)) {}
  };

  template<>
  struct Operation<-1>: Node {
    UNodes items;
    Operation(const Pos *pos, UNodes &&items) noexcept: Node(pos), items(std::move(items)) {}
  };

  template<int N, uint64_t Op>
  struct Operation1 final: Operation<N> {
    using Operation<N>::Operation;
  };

  // arithmetical operations begin
  using ArithAddOper = Operation1<-1, token::symbol("+")>;
  using ArithSubOper = Operation1<-1, token::symbol("-")>;
  using ArithMulOper = Operation1<-1, token::symbol("*")>;
  using ArithDivOper = Operation1<-1, token::symbol("/")>;
  using ArithModOper = Operation1<-1, token::symbol("%")>;
  using ArithPowOper = Operation1<-1, token::symbol("**")>;
  // arithmetical operations end
  // logical operations begin
  using LogicAndOper = Operation1<-1, token::symbol("&&")>;
  using LogicOrOper = Operation1<-1, token::symbol("||")>;
  using LogicNotOper = Operation1<1, token::symbol("!")>;
  using LogicXorOper = Operation1<-1, token::symbol("^^")>;
  // logical operations end
  // bitwise operations begin
  using BitwsAndOper = Operation1<-1, token::symbol("&")>;
  using BitwsOrOper = Operation1<-1, token::symbol("|")>;
  using BitwsNotOper = Operation1<1, token::symbol("~")>;
  using BitwsXorOper = Operation1<-1, token::symbol("^")>;
  using LshOper = Operation1<-1, token::symbol("<<")>;
  using RshOper = Operation1<-1, token::symbol(">>")>;
  using UshOper = Operation1<-1, token::symbol(">>>")>;
  // bitwise operations end
  // compare operations begin
  using CmpEqOper = Operation1<2, token::symbol("==")>;
  using CmpLtOper = Operation1<2, token::symbol("<")>;
  using CmpGtOper = Operation1<2, token::symbol(">")>;
  using CmpLteqOper = Operation1<2, token::symbol("<=")>;
  using CmpGteqOper = Operation1<2, token::symbol(">=")>;
  using CompareOper = Operation1<2, token::symbol("<=>")>;
  // compare operations end
  using AssignOper = Operation1<2, token::symbol("=")>;
  using GetMemberOper = Operation1<-1, token::symbol(".")>;
  using LengthOper = Operation1<1, token::KWD_length>;

  struct SetMemberOper final: Operation<3> {
    using Operation<3>::Operation;
  };
  // operations end
}
