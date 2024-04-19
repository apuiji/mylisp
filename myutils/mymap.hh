#pragma once

#include"myset.hh"

namespace zlt::mymap {
  template<class K, class T>
  using Node = myset::Node<std::pair<K, T>>;

  template<class Comp>
  struct KeyCompare {
    const Comp &comp;
    KeyCompare(const Comp &comp) noexcept: comp(comp) {}
    template<class U, class K, class T>
    auto operator ()(U &&u, const std::pair<K, T> &kt) const noexcept {
      return comp(std::forward<U>(u), kt.first);
    }
  };

  template<class K, class T, class U, class Comp = Compare>
  static inline Node<K, T> *find(const Node<K, T> *node, U &&u, const Comp &comp = {}) noexcept {
    return myset::find(node, std::forward<U>(u), KeyCompare(comp));
  }

  /// @param[out] dest found or inserted node
  /// @return is it inserted
  template<class K, class T, class U, class Supply, class Comp = Compare>
  static inline bool insert(Node<K, T> *&dest, Node<K, T> *&root, U &&u, Supply &&supply, const Comp &comp = {}) {
    return myset::insert(dest, root, std::forward<U>(u), std::forward<Supply>(supply), KeyCompare(comp));
  }

  /// @return is is erased
  template<class K, class T, class U, class Del = std::default_delete<T>, class Comp = Compare>
  static inline bool erase(Node<K, T> *&root, U &&u, Del &&del = {}, const Comp &comp = {}) {
    return myset::erase(root, std::forward<U>(u), std::forward<Del>(del), KeyCompare(comp));
  }
}
