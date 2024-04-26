#pragma once

#include<list>
#include<set>
#include"myutils/mymap.hh"
#include"value.hh"

namespace zlt::mylisp {
  using GlobalDefs = MyMap<const std::string *, Value>;
  using GlobalDef = typename GlobalDefs::Node;

  extern GlobalDefs globalDefs;

  using ObjectPool = std::list<Object *>;

  extern ObjectPool objectPool;
  extern std::set<std::string> strings;

  const std::string *addString(std::string &&s);
  void exec();
  void initGlobalDefs();

  namespace bad {}
}
