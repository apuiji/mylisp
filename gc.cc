#include<algorithm>
#include"object.hh"
#include"vm.hh"

using namespace std;

namespace zlt::mylisp::gc {
  void gc(const Value *args, size_t argc) noexcept {
    for (auto &[name, value] : myiter::range(mymap::begin(globalDefs), mymap::end(globalDefs))) {
      mark(value);
    }
    for (auto a = vm::deferk::begin; a != vm::dsp; ++a) {
      mark(*a);
    }
  }

  void mark(Object *obj) noexcept {
    if (obj->mark) [[unlikely]] {
      return;
    }
    obj->mark = true;
    obj->gcMarkSubjs();
  }

  void mark(Value &value) noexcept {
    if (Object *o; dynamicast(o, value)) {
      mark(o);
    }
  }
}
