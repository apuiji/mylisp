#pragma once

#include"myset.hh"

namespace zlt {
  template<class K, class T>
  using MyMap = MySet<std::pair<K, T>>;
}

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

  template<class K, class T, class Del = std::default_delete<Node<K, T>>>
  static inline void clean(MyMap<K, T> &map, const Del &del = {}) noexcept {
    myset::clean<std::pair<K, T>>(map.root, del);
  }

  template<class K, class T, class U, class Comp = Compare>
  static inline Node<K, T> *find(MyMap<K, T> &map, U &&u, const Comp &comp = {}) noexcept {
    auto node = myset::find<std::pair<K, T>>(map.root, std::forward<U>(u), KeyCompare(comp));
    return static_cast<Node<K, T> *>(node);
  }

  template<class K, class T, class U, class Comp = Compare>
  static inline const Node<K, T> *find(const MyMap<K, T> &map, U &&u, const Comp &comp = {}) noexcept {
    auto node = myset::find<std::pair<K, T>>(map.root, std::forward<U>(u), KeyCompare(comp));
    return static_cast<const Node<K, T> *>(node);
  }

  /// @param[out] dest found or inserted node
  /// @return is it inserted
  template<class K, class T, class U, class Supply, class Comp = Compare>
  static inline bool insert(Node<K, T> *&dest, MyMap<K, T> &map, U &&u, Supply &&supply, const Comp &comp = {}) {
    return myset::insert(dest, map, std::forward<U>(u), std::forward<Supply>(supply), KeyCompare(comp));
  }

  /// @return is is erased
  template<class K, class T, class U, class Del = std::default_delete<Node<K, T>>, class Comp = Compare>
  static inline bool erase(MyMap<K, T> &map, U &&u, Del &&del = {}, const Comp &comp = {}) {
    return myset::erase(map, std::forward<U>(u), std::forward<Del>(del), KeyCompare(comp));
  }

  // iterators begin
  template<class K, class T>
  using Iterator = myset::Iterator<std::pair<K, T>>;

  template<class K, class T, bool Right = true>
  static inline Iterator<K, T> begin(MyMap<K, T> &map) noexcept {
    return myset::begin(map);
  }

  template<class K, class T, bool Right = true>
  static inline Iterator<K, T> end(MyMap<K, T> &map) noexcept {
    return myset::end(map);
  }
  // iterators end
}
