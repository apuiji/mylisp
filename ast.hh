#pragma once

#include<filesystem>
#include<memory>
#include<string>

namespace zlt::mylisp::ast {
  struct Node;

  struct Pos {
    const std::filesystem::path &file;
    int li;
    Pos(const std::filesystem::path &file, int li) noexcept: file(file), li(li) {}
  };

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
