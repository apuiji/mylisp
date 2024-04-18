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
  Node *mostLeft(Node *node) noexcept;
  /// @param node not null
  Node *mostRight(Node *node) noexcept;
  /// @param node not null
  Node *mostTop(Node *node) noexcept;

  /// @param node not null
  template<bool Right>
  static inline Node *mostSide(Node *node) noexcept {
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

  int afterInsert(Node *&root, Node *node) noexcept;
  int beforeErase(Node *&root, Node *node) noexcept;

  template<std::derived_from<Node> T, bool Right = true>
  struct Iterator {
    T *value;
    Iterator(T *value = nullptr) noexcept: value(value) {}
    T *operator ->() const {
      return value;
    }
  };

  template<class T, bool Right>
  static inline bool operator ==(Iterator<T, Right> a, Iterator<T, Right> b) noexcept {
    return a.value == b.value;
  }

  template<class T, bool Right>
  static inline bool operator !=(Iterator<T, Right> a, Iterator<T, Right> b) noexcept {
    return a.value != b.value;
  }

  template<class T, bool Right>
  static inline T &operator *(Iterator<T, Right> &it) noexcept {
    return *it.value;
  }

  template<class T, bool Right>
  static inline auto &operator ++(Iterator<T, Right> &it) noexcept {
    it.value = static_cast<T *>(next<Right>(it.value));
    return it;
  }

  template<class T, bool Right>
  static inline auto operator ++(Iterator<T, Right> &it, int) noexcept {
    auto it1 = it;
    it.value = static_cast<T *>(next<Right>(it.value));
    return it1;
  }

  template<class T, bool Right>
  static inline auto &operator --(Iterator<T, Right> &it) noexcept {
    it.value = next<!Right>(it.value);
    return it;
  }

  template<class T, bool Right>
  static inline auto operator --(Iterator<T, Right> &it, int) noexcept {
    auto it1 = it;
    it.value = next<!Right>(it.value);
    return it1;
  }

  template<std::derived_from<Node> T, bool Right = true>
  static inline auto makeRange(T *root) noexcept {
    struct Range {
      T *beginv;
      Range(T *beginv) noexcept: beginv(beginv) {}
      Iterator<T, Right> begin() const noexcept {
        return beginv;
      }
      Iterator<T, Right> end() const noexcept {
        return nullptr;
      }
    };
    if (root) {
      return Range(static_cast<T *>(mostSide<!Right>(root)));
    } else {
      return Range(nullptr);
    }
  }
}
