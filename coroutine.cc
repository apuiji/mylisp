#include"coroutine.hh"

using namespace std;

namespace zlt::mylisp {
  void *ValueStack::operator new(size_t n, size_t m) {
    void *p = new char[n + m];
    auto q = (ValueStack *) p;
    q->size = m;
    return q;
  }

  Coroutine::Coroutine(unique_ptr<ValueStack> &&valuek) noexcept: valuek(std::move(valuek)), alive(true) {
    valuekBottom = valuek->data;
    valuekTop = valuek->data;
  }
}
