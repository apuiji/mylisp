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
  map<string, pair<void *, Value>> dlibs;
  set<string> fnBodies;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  map<const wstring *, Value, GlobalDefsComp> globalDefs;

  bool GlobalDefsComp::operator ()(const wstring *a, const wstring *b) const noexcept {
    return *a < *b;
  }

  static Value natfn_dlopen(const Value *it, const Value *end);

  int init() {
    #define globalDefn(name) globalDefs[L###name] = natfn_##name
    // strings begin
    globalDefs[constring<'s', 't', 'r', 'c', 'a', 't'>] = natfn_strcat;
    globalDefs[constring<'s', 't', 'r', 'j', 'o', 'i', 'n'>] = natfn_strjoin;
    globalDefs[constring<'s', 't', 'r', 's', 'l', 'i', 'c', 'e'>] = natfn_strslice;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'd'>] = natfn_strtod;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'i'>] = natfn_strtoi;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'l', 'o', 'w', 'e', 'r'>] = natfn_strtolower;
    globalDefs[constring<'s', 't', 'r', 't', 'o', 'u', 'p', 'p', 'e', 'r'>] = natfn_strtoupper;
    globalDefs[constring<'s', 't', 'r', 'v', 'i', 'e', 'w'>] = natfn_strview;
    // strings end
    // regex begin
    globalDefs[constring<'r', 'e', 'g', 'c', 'o', 'm', 'p'>] = natfn_regcomp;
    globalDefs[constring<'r', 'e', 'g', 'e', 'x', 'e', 'c'>] = natfn_regexec;
    // regex end
    // io begin
    globalDefs[constring<'s', 't', 'd', 'o', 'u', 't'>] = gc::neobj(new WriterObj(wcout));
    globalDefs[constring<'s', 't', 'd', 'e', 'r', 'r'>] = gc::neobj(new WriterObj(wcerr));
    globalDefs[constring<'w', 'r', 'i', 't', 'e'>] = natfn_write;
    globalDefs[constring<'o', 'u', 't', 'p', 'u', 't'>] = natfn_output;
    // io end
    globalDefs[constring<'d', 'l', 'o', 'p', 'e', 'n'>] = natfn_dlopen;
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
  static Value dlib(string &name);

  Value natfn_dlopen(const Value *it, const Value *end) {
    string name;
    if (!dlname(name, it, end)) {
      return Null();
    }
    return dlib(name);
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

  Value dlib(string &name) {
    auto it = dlibs.find(name);
    if (it != dlibs.end()) {
      return it->second.second;
    }
    void *dl = dlopen(name.data(), RTLD_LAZY | RTLD_LOCAL);
    if (!dl) {
      return Null();
    }
    auto load = (Value (*)()) dlsym(dl, "mylispLoadDlib");
    if (!load) {
      return Null();
    }
    auto a = load();
    dlibs[std::move(name)] = make_pair(dl, a);
    return a;
  }
}
