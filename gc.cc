#include<algorithm>
#include"object.hh"
#include"vm.hh"

using namespace std;

namespace zlt::mylisp::gc {
  void gc(const Value *args, size_t argc) noexcept {
    for (auto it = vm::defer)
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
