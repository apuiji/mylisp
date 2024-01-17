#include<iostream>
#include"eval.hh"
#include"gc.hh"
#include"io_obj.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  set<string> fnBodies;
  map<const wstring *, Value> globalDefs;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  static const wstring *globDefName(const wchar_t *name) {
    return &*strings.insert(name).first;
  }

  static int natFnWrite(Value *it, Value *end);
  // static int natFnStrcat(Value *it, Value *end);

  int init() {
    {
      auto o = new WriterObj(wcout);
      gc::neobj(o);
      globalDefs[globDefName(L"stdout")] = o;
    }
    {
      auto o = new WriterObj(wcerr);
      gc::neobj(o);
      globalDefs[globDefName(L"stderr")] = o;
    }
    globalDefs[globDefName(L"write")] = natFnWrite;
    // globalDefs[globDefName(L"strcat")] = natFnStrcat;
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

  int natFnWrite(Value *it, Value *end) {
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

  // int natFnStrcat(Value *it, Value *end) {
  //   if (it == end) [[unlikely]] {
  //     return 0;
  //   }
  // }
}
