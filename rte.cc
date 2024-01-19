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
  map<const wchar_t *, Value, GlobalDefsKeyComparator> globalDefs;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  int init() {
    #define globalDefn(name) globalDefs[L###name] = natfn_##name
    // strings begin
    globalDefn(strcat);
    globalDefn(strjoin);
    globalDefn(strslice);
    globalDefn(strtod);
    globalDefn(strtoi);
    globalDefn(strtolower);
    globalDefn(strtoupper);
    globalDefn(strtrim);
    globalDefn(strtriml);
    globalDefn(strtrimr);
    globalDefn(strview);
    // strings end
    // regex begin
    globalDefn(regcomp);
    globalDefn(regexec);
    // regex end
    // io begin
    globalDefs[L"stdout"] = gc::neobj(new WriterObj(wcout));
    globalDefs[L"stderr"] = gc::neobj(new WriterObj(wcerr));
    globalDefn(write);
    globalDefn(output);
    // io end
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
