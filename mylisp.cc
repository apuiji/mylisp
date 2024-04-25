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
}
