#include<dlfcn.h>
#include<filesystem>
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
  map<string, void *> dlibs;
  set<string> fnBodies;
  map<const wchar_t *, Value, GlobalDefsKeyComparator> globalDefs;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  static Value natfn_dlopen(const Value *it, const Value *end);

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
    globalDefn(dlopen);
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

  static bool dlname(string &dest, const Value *it, const Value *end) noexcept;
  static void *dlib(string &name);

  Value natfn_dlopen(const Value *it, const Value *end) {
    string name;
    if (!dlname(name, it, end)) {
      return Null();
    }
    auto dl = dlib(name);
    if (!dl) {
      return Null();
    }
    auto load = (Value (*)()) dlsym(dl, "load");
    if (!load) {
      return Null();
    }
    return load();
  }

  bool dlname(string &dest, const Value *it, const Value *end) noexcept {
    wstring_view sv;
    if (!dynamicast(sv, it, end)) {
      return false;
    }
    filesystem::path path(sv);
    try {
      path = filesystem::canonical(path);
    } catch (...) {
      return false;
    }
    dest = path.string();
    return true;
  }

  void *dlib(string &name) {
    auto it = dlibs.find(name);
    if (it != dlibs.end()) {
      return it->second;
    }
    void *dl = dlopen(name.data(), RTLD_LAZY | RTLD_LOCAL);
    if (!dl) {
      return nullptr;
    }
    dlibs[std::move(name)] = dl;
    return dl;
  }
}
