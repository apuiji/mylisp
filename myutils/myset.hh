#pragma once

#include<compare>
#include<functional>
#include"rbtree.hh"
#include"xyz.hh"

namespace zlt {
  template<class T>
  struct MySet {
    using Value = T;
    rbtree::Node *root;
  };
}

namespace zlt::myset {
  template<class T>
  struct Node: rbtree::Node {
    using Value = T;
    T value;
    Node() = default;
    Node(const T &value): value(value) {}
    Node(T &&value) noexcept: value(std::move(value)) {}
  };

  template<class T, class Del = std::default_delete<Node<T>>>
  void clean(rbtree::Node *node, const Del &del = {}) noexcept {
    if (!node) [[unlikely]] {
      return;
    }
    clean<T>(node->lchild, del);
    clean<T>(node->rchild, del);
    del(static_cast<Node<T> *>(node));
  }

  template<class T, class Del = std::default_delete<Node<T>>>
  static inline void clean(MySet<T> &set, const Del &del = {}) noexcept {
    clean<T>(set.root, del);
  }

  template<class T, class U, class Comp = Compare>
  rbtree::Node *find(const rbtree::Node *node, U &&u, const Comp &comp = {}) noexcept {
    if (!node) [[unlikely]] {
      return nullptr;
    }
    auto diff = comp(std::forward<U>(u), static_cast<const Node<T> *>(node)->value);
    if (std::is_lt(diff)) {
      return find<T>(node->lchild, std::forward<U>(u), comp);
    }
    if (std::is_gt(diff)) {
      return find<T>(node->rchild, std::forward<U>(u), comp);
    }
    return const_cast<rbtree::Node *>(node);
  }

  template<class T, class U, class Comp = Compare>
  static inline Node<T> *find(MySet<T> &set, U &&u, const Comp &comp = {}) noexcept {
    auto node = find<T>(set.root, std::forward<U>(u), comp);
    return static_cast<Node<T> *>(node);
  }

  template<class T, class U, class Comp = Compare>
  static inline const Node<T> *find(const MySet<T> &set, U &&u, const Comp &comp = {}) noexcept {
    auto node = find<T>(set.root, std::forward<U>(u), comp);
    return static_cast<const Node<T> *>(node);
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
  bool insert(Node<T> *&dest, MySet<T> &set, U &&u, Supply &&supply, const Comp &comp = {}) {
    rbtree::Node *parent = nullptr;
    auto &node = findToInsert<T>(parent, set.root, std::forward<U>(u), comp);
    if (node) {
      dest = static_cast<Node<T> *>(node);
      return false;
    }
    dest = supply();
    dest->parent = parent;
    node = dest;
    rbtree::afterInsert(set.root, node);
    return true;
  }

  /// @return is is erased
  template<class T, class U, class Del = std::default_delete<Node<T>>, class Comp = Compare>
  bool erase(MySet<T> &set, U &&u, Del &&del = {}, const Comp &comp = {}) noexcept {
    auto node = find(set.root, std::forward<U>(u), comp);
    if (!node) [[unlikely]] {
      return false;
    }
    rbtree::beforeErase(set.root, node);
    del(static_cast<Node<T> *>(node));
    return true;
  }

  // iterators begin
  template<class T, bool Right = true>
  struct Iterator {
    rbtree::Node *node;
    Iterator(rbtree::Node *node = nullptr) noexcept: node(node) {}
    bool operator ==(const Iterator<T, Right> &it) const noexcept {
      return node == it.node;
    }
    bool operator !=(const Iterator<T, Right> &it) const noexcept {
      return node != it.node;
    }
    T &operator *() noexcept {
      return static_cast<Node<T> *>(node)->value;
    }
    T *operator ->() {
      return &static_cast<Node<T> *>(node)->value;
    }
    Iterator<T, Right> &operator ++() noexcept {
      node = rbtree::next<Right>(node);
      return *this;
    }
    Iterator<T, Right> operator ++(int) noexcept {
      auto it = *this;
      node = rbtree::next<Right>(node);
      return it;
    }
    Iterator<T, Right> &operator --() noexcept {
      node = rbtree::next<!Right>(node);
      return *this;
    }
    Iterator<T, Right> operator --(int) noexcept {
      auto it = *this;
      node = rbtree::next<Right>(node);
      return it;
    }
  };

  template<class T, bool Right = true>
  static inline Iterator<T, Right> begin(MySet<T> &set) noexcept {
    return rbtree::mostSide<!Right>(set.root);
  }

  template<class T, bool Right = true>
  static inline Iterator<T, Right> end(MySet<T> &set) noexcept {
    return nullptr;
  }
  // iterators end
}
