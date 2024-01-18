#include<iostream>
#include<sstream>
#include"eval.hh"
#include"gc.hh"
#include"io.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  set<string> fnBodies;
  map<const wstring *, Value> globalDefs;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  static int setGlobalDef(const wchar_t *name, const Value &value);
  static Value natFnStrcat(const Value *it, const Value *end);
  static Value natFnStrjoin(const Value *it, const Value *end);

  int init() {
    setGlobalDef(L"strcat", natFnStrcat);
    setGlobalDef(L"strjoin", natFnStrjoin);
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

  static int strcat1(wstringstream &dest, const Value *it, const Value *end);

  Value natFnStrcat(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return L"";
    }
    wstringstream ss;
    strcat1(ss, it, end);
    return ss.str();
  }

  int strcat1(wstringstream &dest, const Value *it, const Value *end) {
    write(dest, *it);
    return it + 1 != end ? strcat1(dest, it + 1, end) : 0;
  }

  static int strjoin1(wstringstream &dest, wstring_view sepa, ListObj::ConstIterator it, ListObj::ConstIterator end);

  Value natFnStrjoin(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return L"";
    }
    ListObj *lso;
    if (!dynamicast(lso, *it)) {
      return toStringValue(*it);
    }
    wstring_view sepa;
    if (!(it + 1 != end && dynamicast(sepa, it[1]))) {
      sepa = L"";
    }
    wstringstream ss;
    strjoin1(ss, sepa, lso->list.begin(), lso->list.end());
    return ss.str();
  }

  int strjoin1(wstringstream &dest, wstring_view sepa, ListObj::ConstIterator it, ListObj::ConstIterator end) {
    write(dest, *it);
    if (it + 1 == end) {
      return 0;
    }
    dest << sepa;
    return strjoin1(dest, sepa, it + 1, end);
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
