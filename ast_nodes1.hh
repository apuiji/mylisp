#pragma once

#include<array>
#include<set>
#include"ast_nodes.hh"
#include"ast_token.hh"

namespace zlt::mylisp::ast {
  struct Call final: Node {
    UNode callee;
    UNodes args;
    Call(const char *start, UNode &&callee, UNodes &&args) noexcept:
    Node(start), callee(std::move(callee)), args(std::move(args)) {}
  };

  struct Callee final: Node {
    using Node::Node;
  };

  struct Defer final: Node {
    UNode item;
    Defer(const char *start, UNode &&item) noexcept: Node(start), item(std::move(item)) {}
  };

  struct Forward final: Node {
    UNode callee;
    UNodes args;
    Forward(const char *start, UNode &&callee, UNodes &&args) noexcept:
    Node(start), callee(std::move(callee)), args(std::move(args)) {}
  };

  struct Function final: Node {
    using Defs = std::set<const std::string *>;
    using Params = std::vector<const std::string *>;
    Defs defs;
    Params params;
    UNodes body;
    Function(const char *start, Defs &&defs, Params &&params, UNodes &&body) noexcept:
    Node(start), defs(std::move(defs)), params(std::move(params)), body(std::move(body)) {}
  };

  struct If final: Node {
    UNode cond;
    UNode then;
    UNode elze;
    If(const char *start, UNode &&cond, UNode &&then, UNode &&elze) noexcept:
    Node(start), cond(std::move(cond)), then(std::move(then)), elze(std::move(elze)) {}
  };

  struct Null final: Node {
    using Node::Node;
  };

  struct Number final: Node {
    double value;
    Number(const char *start, double value) noexcept: Node(start), value(value) {}
  };

  struct Return final: Node {
    UNode value;
    Return(const char *start, UNode &&value) noexcept: Node(start), value(std::move(value)) {}
  };

  struct Throw final: Node {
    UNode value;
    Throw(const char *start, UNode &&value) noexcept: Node(start), value(std::move(value)) {}
  };

  struct Try final: Node {
    UNodes body;
    Try(const char *start, UNodes &&body) noexcept: Node(start), body(std::move(body)) {}
  };

  struct Yield final: Node {
    UNode then;
    Yield(const char *start, UNode &&then) noexcept: Node(start), then(std::move(then)) {}
  };

  // operations begin
  template<int N>
  struct Operation: Node {
    std::array<UNode, N> items;
    Operation(const char *start, std::array<UNode, N> &&items) noexcept: Node(start), items(std::move(items)) {}
  };

  template<>
  struct Operation<1>: Node {
    UNode item;
    Operation(const char *start, UNode &&item) noexcept: Node(start), item(std::move(item)) {}
  };

  template<>
  struct Operation<-1>: Node {
    UNodes items;
    Operation(const char *start, UNodes &&items) noexcept: Node(start), items(std::move(items)) {}
  };

  template<int N, int Op>
  struct Operation1 final: Operation<N> {
    using Operation<N>::Operation;
  };

  // arithmetical operations begin
  using ArithAddOper = Operation1<-1, "+"_token>;
  using ArithSubOper = Operation1<-1, "-"_token>;
  using ArithMulOper = Operation1<-1, "*"_token>;
  using ArithDivOper = Operation1<-1, "/"_token>;
  using ArithModOper = Operation1<-1, "%"_token>;
  using ArithPowOper = Operation1<-1, "**"_token>;
  // arithmetical operations end
  // logical operations begin
  using LogicAndOper = Operation1<-1, "&&"_token>;
  using LogicOrOper = Operation1<-1, "||"_token>;
  using LogicNotOper = Operation1<1, "!"_token>;
  using LogicXorOper = Operation1<-1, "^^"_token>;
  // logical operations end
  // bitwise operations begin
  using BitwsAndOper = Operation1<-1, "&"_token>;
  using BitwsOrOper = Operation1<-1, "|"_token>;
  using BitwsNotOper = Operation1<1, "~"_token>;
  using BitwsXorOper = Operation1<-1, "^"_token>;
  using LshOper = Operation1<-1, "<<"_token>;
  using RshOper = Operation1<-1, ">>"_token>;
  using UshOper = Operation1<-1, ">>>"_token>;
  // bitwise operations end
  // compare operations begin
  using CmpEqOper = Operation1<2, "=="_token>;
  using CmpLtOper = Operation1<2, "<"_token>;
  using CmpGtOper = Operation1<2, ">"_token>;
  using CmpLteqOper = Operation1<2, "<="_token>;
  using CmpGteqOper = Operation1<2, ">="_token>;
  using CompareOper = Operation1<2, "<=>"_token>;
  // compare operations end
  using AssignOper = Operation1<2, "="_token>;
  using GetMemberOper = Operation1<-1, "."_token>;
  using LengthOper = Operation1<1, "length"_token>;
  using SequenceOper = Operation1<-1, ","_token>;

  struct SetMemberOper final: Operation<3> {
    using Operation<3>::Operation;
  };
  // operations end
}
