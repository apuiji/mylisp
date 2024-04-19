#pragma once

#include<compare>
#include<concepts>
#include<memory>

namespace zlt {
  #ifdef __cpp_lib_three_way_comparison

  using Compare = std::compare_three_way;

  #else

  struct Compare {
    template<class T, class U>
    auto operator ()(T &&t, U &&u) const {
      return t <=> u;
    }
  };

  #endif

  template<class ...T>
  struct Dynamicastable {
    template<class U>
    requires (std::is_pointer_v<U>)
    bool operator ()(const U u) noexcept {
      return u && (dynamic_cast<const T*>(u) || ...);
    }
    template<class U>
    requires (!std::is_pointer_v<U>)
    bool operator ()(const U &u) noexcept {
      return operator ()(&u);
    }
  };

  template<class T>
  struct EscRAII {
    char data[sizeof(T)];
    template<class ...Args>
    EscRAII(Args &&...args) {
      std::construct_at((T *) data, std::forward<Args>(args)...);
    }
    T *get() const noexcept {
      return (T *) data;
    }
    T &operator *() const noexcept {
      return *get();
    }
    T *operator ->() const noexcept {
      return get();
    }
    void operator delete(void *p) {
      std::destroy_at(((EscRAII<T> *) p)->get());
    }
  };

  template<class T>
  static inline auto makeEscRAII(T &&t) {
    return EscRAII<T>(std::move(t));
  }

  template<class T, class ...U>
  static constexpr bool isAnyOf = (std::is_same_v<T, U> || ...);

  template<class T, class ...U>
  concept AnyOf = isAnyOf<T, U...>;

  template<std::invocable T>
  static inline auto makeGuard(T &&t) noexcept {
    struct Guard {
      T t;
      Guard(T &&t) noexcept: t(std::move(t)) {}
      ~Guard() {
        t();
      }
    };
    return Guard(std::move(t));
  }

  template<std::invocable T>
  static inline auto makeGuard(T &&t, bool &on) noexcept {
    struct Guard {
      T t;
      bool &on;
      Guard(T &&t, bool &on) noexcept: t(std::move(t)), on(on) {}
      ~Guard() {
        if (on) {
          t();
        }
      }
    };
    return Guard(std::move(t), on);
  }

  /// overloaded function resolve
  template<class R, class ...Args>
  static inline constexpr auto ofr(R (*f)(Args...)) noexcept {
    return f;
  }

  template<class T>
  static inline T remove(T &t) noexcept {
    return std::move(t);
  }
}
