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
  Node<T> *&findToInsert(Node<T> *&parent, Node<T> *&node, U &&u, const Comp &comp = {}) noexcept {
    if (!node) [[unlikely]] {
      return node;
    }
    auto diff = comp(std::forward<U>(u), node->value);
    if (std::is_lt(diff)) {
      parent = node;
      return findToInsert(parent, static_cast<Node<T> *&>(node->lchild), std::forward<U>(u), comp);
    }
    if (std::is_gt(diff)) {
      parent = node;
      return findToInsert(parent, static_cast<Node<T> *&>(node->rchild), std::forward<U>(u), comp);
    }
    return node;
  }

  /// @param[out] dest found or inserted node
  /// @return is it inserted
  template<class T, class U, class Supply, class Comp = Compare>
  bool insert(Node<T> *&dest, Node<T> *&root, U &&u, Supply &&supply, const Comp &comp = {}) {
    Node<T> *parent = nullptr;
    auto &node = findToInsert(parent, root, std::forward<U>(u), comp);
    if (node) {
      dest = node;
      return false;
    }
    dest = supply();
    dest->parent = parent;
    node = dest;
    rbtree::afterInsert(root, node);
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
