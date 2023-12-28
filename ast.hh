#pragma once

#include<filesystem>
#include<memory>
#include<string>
#include<utility>
#include<vector>

namespace zlt::mylisp::ast {
  struct Node;

  using Pos = std::pair<const std::filesystem::path *, int>;
  using UNode = std::unique_ptr<Node>;
  using UNodes = std::vector<UNode>;

  struct Node {
    const Pos *pos;
    UNode next;
    Node(const Pos *pos = nullptr) noexcept: pos(pos) {}
    virtual ~Node() = default;
  };
}
