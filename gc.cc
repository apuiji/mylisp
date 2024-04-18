#include<algorithm>
#include"object.hh"

using namespace std;

namespace zlt::mylisp::gc {
  static int step;
  static Object *blacks;
  static Object *grays;
  static Object *whites;

  static void gc0();
  static void gc1();
  static void gc2();

  void gc() {
    if (step == 0) {
      gc0();
    } else if (step == 1) {
      gc1();
    } else {
      gc2();
    }
    yield();
  }

  void gc0() {
    whites = std::exchange(blacks, nullptr);
    for (auto a = whites; a; a = a->next) {
      a->color = Object::WHITE;
    }
    for_each(globalDefs.begin(), globalDefs.end(), grayValue);
    for (auto &a : coroutines) {
      grayValue(a.ax);
      for_each(a.valuek, a.sp, grayValue);
      for_each(a.deferk, a.dsp, grayValue);
    }
    step = 1;
  }

  void gc1() {
    if (!grays) [[unlikely]] {
      step = 2;
      return;
    }
    auto a = std::exchange(grays, nullptr);
    while (a) {
      auto next = a->next;
      a->gcGraySubjs();
      blackObj(a);
      a = next;
    }
  }

  void gc2() {
    if (!whites) [[unlikely]] {
      step = 0;
      return;
    }
    auto a = whites;
    while (a) {
      auto next = a->next;
      delete a;
      a = next;
    }
    step = 0;
  }
}
