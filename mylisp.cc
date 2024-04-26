#include"io.hh"
#include"mylisp.hh"

using namespace std;

namespace zlt::mylisp {
  GlobalDefs globalDefs;
  ObjectPool objectPool;
  set<string> strings;

  const string *addString(string &&s) {
    auto [it, b] = strings.insert(std::move(s));
    return &*it;
  }

  static void setGlobalDef(std::string &&name, const Value &value);

  static inline void setObjGlobalDef(std::string &&name, Object *obj) {
    objectPool.push_back(obj);
    setGlobalDef(std::move(name), obj);
  }

  void initGlobalDefs() {
    setObjGlobalDef("stdin", new io::StdinObj());
    setObjGlobalDef("stdout", new io::StdoutObj(cout));
    setObjGlobalDef("stderr", new io::StdoutObj(cerr));
    setGlobalDef("input", io::input);
    setGlobalDef("output", io::output);
  }

  void setGlobalDef(std::string &&name, const Value &value) {
    auto [it, b] = strings.insert(std::move(name));
    GlobalDef *gd;
    if (mymap::insert(gd, globalDefs, &*it, [] () { return new GlobalDef; })) {
      gd->value.first = &*it;
    }
    gd->value.second = value;
  }
}
