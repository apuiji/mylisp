#include<filesystem>
#include<iostream>
#include<sstream>
#include"eval.hh"
#include"gc.hh"
#include"io.hh"
#include"myccutils/dl.hh"
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
  set<string> strings;

  mymap::Map<const string *, Value, GlobalDefsComp> globalDefs;

  static NativeFunction natfn_dlopen;
  static NativeFunction natfn_dlerror;

  int init() {
    // strings begin
    globalDefs[constring<'c', 'h', 'a', 'r', 'c', 'o', 'd', 'e'>] = natfn_charcode;
    globalDefs[constring<'c', 'h', 'a', 'r', 'c', 'o', 'd', 'e', 's'>] = natfn_charcodes;
    globalDefs[constring<'f', 'r', 'o', 'm', 'c', 'h', 'a', 'r', 'c', 'o', 'd', 'e'>] = natfn_fromcharcode;
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
    globalDefs[constring<'s', 't', 'd', 'i', 'n'>] = neobj<InputObj>(cin);
    globalDefs[constring<'s', 't', 'd', 'o', 'u', 't'>] = neobj<OutputObj>(cout);
    globalDefs[constring<'s', 't', 'd', 'e', 'r', 'r'>] = neobj<OutputObj>(cerr);
    globalDefs[constring<'g', 'e', 't', 'c'>] = natfn_getc;
    globalDefs[constring<'o', 'u', 't', 'p', 'u', 't'>] = natfn_output;
    // io end
    globalDefs[constring<'d', 'l', 'o', 'p', 'e', 'n'>] = natfn_dlopen;
    globalDefs[constring<'d', 'l', 'e', 'r', 'r', 'o', 'r'>] = natfn_dlerror;
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

  Value natfn_dlopen(const Value *it, const Value *end) {
    string path;
    if (!modCanonicalPath(path, it, end)) {
      return Null();
    }
    return loadMod(path);
  }

  bool modCanonicalPath(string &dest, const Value *it, const Value *end) noexcept {
    string_view sv;
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
    auto dl = dl::open(path);
    if (!dl) {
      return Null();
    }
    bool no = true;
    auto g = makeGuard([dl] () { dl::close(dl); }, no);
    auto exp0rt = dl::funct<int, Value &>(dl, "mylispExport");
    if (!exp0rt) {
      return Null();
    }
    Value mod;
    exp0rt(mod);
    mods[std::move(path)] = mod;
    no = false;
    staticast(aaa, mod);
    return mod;
  }

  Value natfn_dlerror(const Value *it, const Value *end) {
    auto what = dlerror();
    if (what) {
      return string(what);
    } else {
      return Null();
    }
  }
}
