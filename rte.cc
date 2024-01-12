#include<iostream>
#include"eval.hh"
#include"gc.hh"
#include"io_object.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  set<filesystem::path> files;
  set<string> fnBodies;
  map<const wstring *, Value> globalDefs;
  ItCoroutine itCoroutine;
  map<const wstring *, Macro> macros;
  set<ast::Pos> positions;
  set<wstring> strings;

  int init() {
    {
      auto &id = *strings.insert(L"stdout").first;
      auto o = new WriterObj(wcout);
      gc::neobj(o);
      globalDefs[&id] = o;
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
}
