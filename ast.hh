#pragma once

#include<filesystem>
#include<memory>
#include<string>
#include<utility>

namespace zlt::mylisp::ast {
  struct Node;

  using Pos = std::pair<const std::filesystem::path *, int>;
  using UNode = std::unique_ptr<Node>;

  struct Node {
    const Pos *pos;
    UNode next;
    Node(const Pos *pos = nullptr) noexcept: pos(pos) {}
    virtual ~Node() = default;
  };

  static inline int shift(UNode &dest, UNode &src) noexcept {
    dest = std::move(src);
    src = std::move(dest->next);
    return 0;
  }
}
