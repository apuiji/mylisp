#pragma once

#include<filesystem>
#include<memory>
#include<string>
#include"xyz.hh"

namespace zlt::mylisp::ast {
  struct Node;

  struct Pos {
    std::filesystem::path *path;
    int li;
  };

  using UNode = std::unique_ptr<Node>;

  struct Node {
    const Pos *pos;
    UNode next;
    Node(const Pos *pos = nullptr) noexcept: pos(pos) {}
    virtual ~Node() = default;
  };

  static inline UNode shift(UNode &src) noexcept {
    UNode u = std::move(src);
    src = std::move(u->next);
    return std::move(u);
  }
}
