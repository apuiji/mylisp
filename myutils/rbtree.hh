#pragma once

#include<concepts>

namespace zlt::rbtree {
  struct Node {
    Node *parent = nullptr;
    Node *children[0];
    Node *lchild = nullptr;
    Node *rchild = nullptr;
    bool red = true;
  };

  /// @param node not null
  Node *mostLeft(const Node *node) noexcept;
  /// @param node not null
  Node *mostRight(const Node *node) noexcept;
  /// @param node not null
  Node *mostTop(const Node *node) noexcept;

  /// @param node not null
  template<bool Right>
  static inline Node *mostSide(const Node *node) noexcept {
    if constexpr (Right) {
      return mostRight(node);
    } else {
      return mostLeft(node);
    }
  }

  /// @param node not null
  Node *nextLeft(const Node *node) noexcept;
  /// @param node not null
  Node *nextRight(const Node *node) noexcept;

  /// @param node not null
  template<bool Right>
  static inline Node *next(const Node *node) noexcept {
    if constexpr (Right) {
      return nextRight(node);
    } else {
      return nextLeft(node);
    }
  }

  /// @param node not null, right child not null also
  /// @return new axis node
  Node *leftRotate(Node *node) noexcept;
  /// @param node not null, left child not null also
  /// @return new axis node
  Node *rightRotate(Node *node) noexcept;

  /// @param node not null, other side child not null also
  /// @return new axis node
  template<bool Right>
  static inline Node *rotate(Node *node) noexcept {
    if constexpr (Right) {
      return rightRotate(node);
    } else {
      return leftRotate(node);
    }
  }

  void afterInsert(Node *&root, Node *node) noexcept;
  void beforeErase(Node *&root, Node *node) noexcept;
}
