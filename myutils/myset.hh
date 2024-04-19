#pragma once

#include<compare>
#include<functional>
#include"rbtree.hh"
#include"xyz.hh"

namespace zlt::myset {
  template<class T>
  struct Node: rbtree::Node {
    using Value = T;
    T value;
    Node() = default;
    Node(const T &value): value(value) {}
    Node(T &&value) noexcept: value(std::move(value)) {}
  };

  template<class T, class Del = std::default_delete<T>>
  int clean(Node<T> *node, const Del &del = {}) noexcept {
    if (!node) [[unlikely]] {
      return 0;
    }
    clean(static_cast<Node<T> *>(node->lchild), del);
    clean(static_cast<Node<T> *>(node->rchild), del);
    del(node);
    return 0;
  }

  template<class T, class U, class Comp = Compare>
  Node<T> *find(const Node<T> *node, U &&u, const Comp &comp = {}) noexcept {
    if (!node) [[unlikely]] {
      return nullptr;
    }
    auto diff = comp(std::forward<U>(u), node->value);
    if (std::is_lt(diff)) {
      return find(static_cast<Node<T> *>(node->lchild), std::forward<U>(u), comp);
    }
    if (std::is_gt(diff)) {
      return find(static_cast<Node<T> *>(node->rchild), std::forward<U>(u), comp);
    }
    return const_cast<Node<T> *>(node);
  }

  /// @param[out] parent initialized by null, the parent node of found
  /// @return not null when already exists
  template<class T, class U, class Comp = Compare>
  rbtree::Node *&findToInsert(rbtree::Node *&parent, rbtree::Node *&node, U &&u, const Comp &comp = {}) noexcept {
    if (!node) [[unlikely]] {
      return node;
    }
    auto diff = comp(std::forward<U>(u), static_cast<Node<T> *>(node)->value);
    if (std::is_lt(diff)) {
      parent = node;
      return findToInsert<T>(parent, node->lchild, std::forward<U>(u), comp);
    }
    if (std::is_gt(diff)) {
      parent = node;
      return findToInsert<T>(parent, node->rchild, std::forward<U>(u), comp);
    }
    return node;
  }

  /// @param[out] dest found or inserted node
  /// @return is it inserted
  template<class T, class U, class Supply, class Comp = Compare>
  bool insert(Node<T> *&dest, Node<T> *&root, U &&u, Supply &&supply, const Comp &comp = {}) {
    rbtree::Node *parent = nullptr;
    rbtree::Node *root1 = root;
    auto &node = findToInsert<T>(parent, root1, std::forward<U>(u), comp);
    if (node) {
      dest = static_cast<Node<T> *>(node);
      return false;
    }
    dest = supply();
    dest->parent = parent;
    if (node == root1) {
      root = dest;
    } else {
      node = dest;
    }
    rbtree::afterInsert(root1, node);
    return true;
  }

  /// @return is is erased
  template<class T, class U, class Del = std::default_delete<T>, class Comp = Compare>
  bool erase(Node<T> *&root, U &&u, Del &&del = {}, const Comp &comp = {}) {
    auto node = find(root, std::forward<U>(u), comp);
    if (!node) [[unlikely]] {
      return false;
    }
    rbtree::beforeErase(root, node);
    del(node);
    return true;
  }
}
