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

  static inline int replace(UNode &dest, UNode &src) noexcept {
    src->next = std::move(dest->next);
    dest = std::move(src);
    return 0;
  }

  static inline int replace(UNode &dest, UNode &&src) noexcept {
    return replace(dest, src);
  }

  int ast(UNode &src);
  UNode shift(UNode &src) noexcept;

  int pos2str(std::wstring &dest, const Pos &src);
}
