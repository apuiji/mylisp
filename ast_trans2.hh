#pragma once

#include"ast_trans1.hh"

namespace zlt::mylisp::ast {
  int trans2(UNode &src);

  struct Function2 final: Node {
    UNode body;
    UNode inputClosure;
    Function2(const Pos *pos, UNode &&body, UNode &&inputClosure) noexcept:
    Node(pos), body(std::move(body)), inputClosure(std::move(inputClosure)) {}
  };

  struct GetIndirectOper final: Operation<1> {
    using Operation<1>::Operation;
  };

  struct InputClosure final: Node {
    const std::wstring *name;
    Reference ref;
    InputClosure(const std::wstring *name, const Reference &ref) noexcept: name(name), ref(ref) {}
  };

  struct MakeIndirect final: Node {
    using Node::Node;
  };

  struct SetIndirectOper final: Operation<2> {
    using Operation<2>::Operation;
  };
}
