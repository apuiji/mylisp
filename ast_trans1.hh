#pragma once

#include<map>
#include"ast_trans.hh"

namespace zlt::mylisp::ast {
  int trans1(UNode &src);

  struct Argument final: Node {
    size_t index;
    Argument(size_t index) noexcept: index(index) {}
  };

  struct CleanArguments final: Node {
    using Node::Node;
  };

  struct Reference {
    enum {
      LOCAL_SCOPE,
      CLOSURE_SCOPE,
      GLOBAL_SCOPE
    };
    size_t scope;
    const std::string *name;
    Reference() noexcept = default;
    Reference(size_t scope, const std::string *name) noexcept: scope(scope), name(name) {}
  };

  struct Function1 final: Node {
    /// indirect definitions
    std::set<const std::string *> indefs;
    std::map<const std::string *, Reference> closureDefs;
    UNode body;
    Function1(
      const Pos *pos,
      std::set<const std::string *> &&indefs,
      std::map<const std::string *, Reference> &&closureDefs,
      UNode &&body
    ) noexcept: Node(pos), indefs(std::move(indefs)), closureDefs(std::move(closureDefs)), body(std::move(body)) {}
  };

  struct Reference1 final: Node, Reference {
    Reference1(const Pos *pos, const Reference &ref) noexcept: Node(pos), Reference(ref) {}
  };
}
