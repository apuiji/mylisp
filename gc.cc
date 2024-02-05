#include<algorithm>
#include"gc.hh"
#include"object.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::gc {
  enum {
    START_STEP,
    NEXT_GRAYS_STEP,
    CLEAN_WHITES_STEP
  };

  static int step;

  static Object *whites;
  static Object *grays;
  Object *blacks;

  static int whiteBlacks(Object *o) noexcept;
  static int firstGrays(ItCoroutine it, ItCoroutine end) noexcept;
  static int nextGrays(Object *o) noexcept;
  static int cleanWhites(Object *o) noexcept;

  int gc() noexcept {
    switch (step) {
      case START_STEP: {
        whiteBlacks(blacks);
        whites = exchange(blacks, nullptr);
        firstGrays(rte::coroutines.begin(), rte::coroutines.end());
        for (auto &m : rte::mods) {
          grayValue(m.second);
        }
        step = NEXT_GRAYS_STEP;
        return 0;
      }
      case NEXT_GRAYS_STEP: {
        if (grays) {
          Object *greys = exchange(grays, nullptr);
          return nextGrays(greys);
        } else {
          step = CLEAN_WHITES_STEP;
          return gc();
        }
      }
      default: {
        cleanWhites(whites);
        step = START_STEP;
        return 0;
      }
    }
  }

  int whiteBlacks(Object *o) noexcept {
    if (!o) [[unlikely]] {
      return 0;
    }
    Object *next = o->next;
    o->color = WHITE_COLOR;
    return whiteBlacks(next);
  }

  static int grayLocalDefs(map<const string *, Value> &defs) noexcept {
    for (auto &p : defs) {
      grayValue(p.second);
    }
    return 0;
  }

  int firstGrays(ItCoroutine it, ItCoroutine end) noexcept {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (!it->alive) {
      return firstGrays(rte::coroutines.erase(it), end);
    }
    grayValue(it->value);
    for_each(it->valuek->begin(), it->valuekTop, grayValue);
    for_each(it->localDefsk.begin(), it->localDefsk.end(), grayLocalDefs);
    for_each(it->deferk.begin(), it->deferk.end(), grayValue);
    return firstGrays(++it, end);
  }

  static int take(Object *&set, Object *o) noexcept;

  int grayObj(Object *o) noexcept {
    if (o->color == WHITE_COLOR) {
      take(whites, o);
      o->color = GRAY_COLOR;
      put(grays, o);
    }
    return 0;
  }

  int grayValue(const Value &v) noexcept {
    Object *o;
    dynamicast(o, v);
    if (o) {
      grayObj(o);
    }
    return 0;
  }

  int take(Object *&set, Object *o) noexcept {
    if (o->prev) {
      o->prev->next = o->next;
    } else {
      set = o->next;
    }
    if (o->next) {
      o->next->prev = o->prev;
    }
    return 0;
  }

  int put(Object *&dest, Object *o) noexcept {
    o->prev = nullptr;
    o->next = dest;
    if (dest) {
      dest->prev = o;
    }
    dest = o;
    return 0;
  }

  int nextGrays(Object *o) noexcept {
    if (!o) [[unlikely]] {
      return 0;
    }
    Object *next = o->next;
    o->color = BLACK_COLOR;
    put(blacks, o);
    o->graySubjs();
    return nextGrays(next);
  }

  int cleanWhites(Object *o) noexcept {
    if (!o) [[unlikely]] {
      return 0;
    }
    auto next = o->next;
    delete o;
    return cleanWhites(next);
  }
}
