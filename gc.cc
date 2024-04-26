#include<algorithm>
#include"gc.hh"
#include"mylisp.hh"
#include"object.hh"
#include"vm.hh"

using namespace std;

namespace zlt::mylisp::gc {
  static void sweep(ObjectPool::iterator it, ObjectPool::iterator end) noexcept;

  void gc(const Value *args, size_t argc) noexcept {
    for_each(mymap::begin(globalDefs), mymap::end(globalDefs), [] (auto &p) { mark(p.second); });
    for_each(vm::deferk::begin, vm::dsp, OFR<Value &> {}(mark));
    sweep(objectPool.begin(), objectPool.end());
    vm::deferk::push(&gc);
  }

  void sweep(ObjectPool::iterator it, ObjectPool::iterator end) noexcept {
    if (it == end) [[unlikely]] {
      return;
    }
    if ((**it).mark) {
      (**it).mark = false;
      sweep(++it, end);
    } else {
      delete *it;
      sweep(objectPool.erase(it), end);
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
