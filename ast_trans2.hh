#pragma once

#include"ast_trans1.hh"

namespace zlt::mylisp::ast {
  int trans2(UNode &src);

  struct GetPointerOper final: Operation<1> {
    using Operation<1>::Operation;
  };

  struct MakePointer final: Node {
    using Node::Node;
  };

  struct SetPointerOper final: Operation<2> {
    using Operation<2>::Operation;
  };
}
