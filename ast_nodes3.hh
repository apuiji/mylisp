#pragma once

#include"ast_nodes2.hh"

namespace zlt::mylisp::ast {
  struct Function2 final: Node {
    UNodes body;
    UNodes inputClosures;
    Function2(const char *start, UNodes &&body, UNodes &&inputClosures) noexcept:
    Node(start), body(std::move(body)), inputClosures(std::move(inputClosures)) {}
  };

  struct GetIndirectOper final: Operation<1> {
    using Operation<1>::Operation;
  };

  struct InputClosure final: Node {
    const std::string *name;
    Reference ref;
    InputClosure(const std::string *name, const Reference &ref) noexcept: name(name), ref(ref) {}
  };

  struct MakeIndirect final: Node {
    using Node::Node;
  };

  struct SetIndirectOper final: Operation<2> {
    using Operation<2>::Operation;
  };
}
