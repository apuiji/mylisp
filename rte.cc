#include<iostream>
#include<sstream>
#include"eval.hh"
#include"gc.hh"
#include"io.hh"
#include"regexs.hh"
#include"rte.hh"
#include"strings.hh"

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  set<string> fnBodies;
  map<const wstring *, Value> globalDefs;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  static int setGlobalDef(const wchar_t *name, const Value &value);

  int init() {
    // strings begin
    setGlobalDef(L"strcat", natFnStrcat);
    setGlobalDef(L"strjoin", natFnStrjoin);
    setGlobalDef(L"strslice", natFnStrslice);
    setGlobalDef(L"strtod", natFnStrtod);
    setGlobalDef(L"strtoi", natFnStrtoi);
    setGlobalDef(L"strtolower", natFnStrtolower);
    setGlobalDef(L"strtoupper", natFnStrtoupper);
    setGlobalDef(L"strview", natFnStrview);
    // strings end
    // regex begin
    setGlobalDef(L"regcomp", natFnRegcomp);
    setGlobalDef(L"regexec", natFnRegexec);
    // regex end
    // io begin
    setGlobalDef(L"stdout", gc::neobj(new WriterObj(wcout)));
    setGlobalDef(L"stderr", gc::neobj(new WriterObj(wcerr)));
    setGlobalDef(L"write", natFnWrite);
    setGlobalDef(L"output", natFnOutput);
    // io end
    return 0;
  }

  int setGlobalDef(const wchar_t *name, const Value &value) {
    auto name1 = &*strings.insert(name).first;
    globalDefs[name1] = value;
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
