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
  ItCoroutine itCoroutine;
  set<wstring> strings;

  map<const wstring *, Value, GlobalDefsComp> globalDefs;

  bool GlobalDefsComp::operator ()(const wstring *a, const wstring *b) const noexcept {
    return *a < *b;
  }

  int init() {
    #define globalDefn(name) globalDefs[L###name] = natfn_##name
    // strings begin
    globalDefs[constring<'s', 't', 'r', 'c', 'a', 't'>] = natFnStrcat;
    globalDefs[constring<'s', 't', 'r', 'j', 'o', 'i', 'n'>] = natFnStrjoin;
    globalDefs[constring<'s', 't', 'r', 's', 'l', 'i', 'c', 'e'>] = natFnStrslice;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'd'>] = natFnStrtod;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'i'>] = natFnStrtoi;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'l', 'o', 'w', 'e', 'r'>] = natFnStrtolower;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'u', 'p', 'p', 'e', 'r'>] = natFnStrtoupper;
    globalDefs[constring<'s', 't', 'r', 'v', 'i', 'e', 'w'>] = natFnStrview;
    // strings end
    // regex begin
    globalDefs[constring<'r', 'e', 'g', 'c', 'o', 'm', 'p'>] = natFnRegcomp;
    globalDefs[constring<'r', 'e', 'g', 'e', 'x', 'e', 'c'>] = natFnRegexec;
    // regex end
    // io begin
    globalDefs[constring<'s', 't', 'd', 'o', 'u', 't'>] = gc::neobj(new WriterObj(wcout));
    globalDefs[constring<'s', 't', 'd', 'e', 'r', 'r'>] = gc::neobj(new WriterObj(wcerr));
    globalDefs[constring<'w', 'r', 'i', 't', 'e'>] = natFnWrite;
    globalDefs[constring<'o', 'u', 't', 'p', 'u', 't'>] = natFnOutput;
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
