#include<filesystem>
#include<iostream>
#include<sstream>
#include"eval.hh"
#include"gc.hh"
#include"io.hh"
#include"myccutils/constr.hh"
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

  GlobalDef *globalDefs;

  int globalDefsComp(const std::string *a, const std::string *b) noexcept {
    return a->compare(*b);
  }

  static NativeFunction natfn_dlopen;
  static NativeFunction natfn_dlerror;

  static int setGlobalDef(const string &name, const Value &value) {
    auto [slot, parent] = mymap::findToInsert(globalDefs, &name, globalDefsComp);
    if (!*slot) {
      *slot = new GlobalDef(GlobalDef::Value(&name, value));
      (**slot).parent = parent;
    }
    return 0;
  }

  int init() {
    // strings begin
    setGlobalDef(constString<'c', 'h', 'a', 'r', 'c', 'o', 'd', 'e'>, natfn_charcode);
    setGlobalDef(constString<'c', 'h', 'a', 'r', 'c', 'o', 'd', 'e', 's'>, natfn_charcodes);
    setGlobalDef(constString<'f', 'r', 'o', 'm', 'c', 'h', 'a', 'r', 'c', 'o', 'd', 'e'>, natfn_fromcharcode);
    setGlobalDef(constString<'s', 't', 'r', 'c', 'a', 't'>, natfn_strcat);
    setGlobalDef(constString<'s', 't', 'r', 'j', 'o', 'i', 'n'>, natfn_strjoin);
    setGlobalDef(constString<'s', 't', 'r', 's', 'l', 'i', 'c', 'e'>, natfn_strslice);
    setGlobalDef(constString<'s', 't', 'r', 't', 'o', 'd'>, natfn_strtod);
    setGlobalDef(constString<'s', 't', 'r', 't', 'o', 'i'>, natfn_strtoi);
    setGlobalDef(constString<'s', 't', 'r', 't', 'o', 'l', 'o', 'w', 'e', 'r'>, natfn_strtolower);
    setGlobalDef(constString<'s', 't', 'r', 't', 'o', 'u', 'p', 'p', 'e', 'r'>, natfn_strtoupper);
    setGlobalDef(constString<'s', 't', 'r', 'v', 'i', 'e', 'w'>, natfn_strview);
    // strings end
    // regex begin
    setGlobalDef(constString<'r', 'e', 'g', 'c', 'o', 'm', 'p'>, natfn_regcomp);
    setGlobalDef(constString<'r', 'e', 'g', 'e', 'x', 'e', 'c'>, natfn_regexec);
    // regex end
    // io begin
    setGlobalDef(constString<'s', 't', 'd', 'i', 'n'>, neobj<InputObj>(cin));
    setGlobalDef(constString<'s', 't', 'd', 'o', 'u', 't'>, neobj<OutputObj>(cout));
    setGlobalDef(constString<'s', 't', 'd', 'e', 'r', 'r'>, neobj<OutputObj>(cerr));
    setGlobalDef(constString<'g', 'e', 't', 'c'>, natfn_getc);
    setGlobalDef(constString<'o', 'u', 't', 'p', 'u', 't'>, natfn_output);
    // io end
    setGlobalDef(constString<'d', 'l', 'o', 'p', 'e', 'n'>, natfn_dlopen);
    setGlobalDef(constString<'d', 'l', 'e', 'r', 'r', 'o', 'r'>, natfn_dlerror);
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
    return mod;
  }

  Value natfn_dlerror(const Value *it, const Value *end) {
    auto what = dl::error();
    if (what) {
      return string(what);
    } else {
      return Null();
    }
  }
}
