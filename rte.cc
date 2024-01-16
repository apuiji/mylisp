#include<iostream>
#include"eval.hh"
#include"gc.hh"
#include"io_object.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  set<string> fnBodies;
  map<const wstring *, Value> globalDefs;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  static int nativeFnWrite(Value *it, Value *end);

  int init() {
    {
      auto &name = *strings.insert(L"stdout").first;
      auto o = new WriterObj(wcout);
      gc::neobj(o);
      globalDefs[&name] = o;
    }
    {
      auto &name = *strings.insert(L"write").first;
      NativeFunction *fp = nativeFnWrite;
      globalDefs[&name] = fp;
    }
    return 0;
  }

  int yield() {
    ++itCoroutine;
    if (itCoroutine == coroutines.end()) {
      gc::gc();
      itCoroutine = coroutines.begin();
      if (itCoroutine == coroutines.end()) {
        return 0;
      }
    }
    Frame f = itCoroutine->framek.back();
    itCoroutine->framek.pop_back();
    return eval(f.prevNext, f.prevEnd);
  }

  int nativeFnWrite(Value *it, Value *end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    WriterObj *wo;
    if (!dynamicast(wo, *it)) [[unlikely]] {
      return 0;
    }
    if (it + 1 == end) [[unlikely]] {
      return 0;
    }
    return write(wo->ostream, it[1]);
  }
}
