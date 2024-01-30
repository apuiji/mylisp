#include<filesystem>
#include<iostream>
#include<sstream>
#include"eval.hh"
#include"gc.hh"
#include"io.hh"
#include"regexs.hh"
#include"rte.hh"
#include"strings.hh"

#ifdef __WIN32__
#include<libloaderapi.h>
#else
#include<dlfcn.h>
#endif

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  map<string, Value> mods;
  set<string> fnBodies;
  ItCoroutine itCoroutine;
  set<wstring> strings;

  mymap::Map<const wstring *, Value, GlobalDefsComp> globalDefs;

  static Value natfn_import(const Value *it, const Value *end);

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
    globalDefs[constring<'i', 'm', 'p', 'o', 'r', 't'>] = natfn_import;
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

  static bool modCanonicalPath(string &dest, const Value *it, const Value *end) noexcept;
  static Value loadMod(string &path);

  Value natfn_import(const Value *it, const Value *end) {
    string path;
    if (!modCanonicalPath(path, it, end)) {
      return Null();
    }
    return loadMod(path);
  }

  bool modCanonicalPath(string &dest, const Value *it, const Value *end) noexcept {
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

  Value loadMod(string &path) {
    auto it = mods.find(path);
    if (it != mods.end()) {
      return it->second;
    }
    #ifdef __WIN32__
    #define dlopen(path, ignore) LoadLibraryA(path)
    #define dlclose FreeLibrary
    #define dlsym GetProcAddress
    #endif
    void *dl = dlopen(path.data(), RTLD_LAZY | RTLD_LOCAL);
    if (!dl) {
      return Null();
    }
    bool no = true;
    auto g = makeGuard([dl] () { dlclose(dl); }, no);
    auto exp0rt = (Value (*)()) dlsym(dl, "mylispExport");
    #undef dlopen
    #undef dlclose
    #undef dlsym
    if (!exp0rt) {
      return Null();
    }
    auto mod = exp0rt();
    mods[std::move(path)] = mod;
    no = false;
    return mod;
  }
}
