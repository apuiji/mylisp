#include<algorithm>
#include"gc.hh"
#include"mylisp.hh"
#include"object.hh"
#include"vm.hh"

using namespace std;

namespace zlt::mylisp::gc {
  void gc(const Value *args, size_t argc) noexcept {
    {
      auto it = mymap::begin(globalDefs);
      auto end = mymap::end(globalDefs);
      for (; it != end; ++it) {
        mark(it->second);
      }
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
