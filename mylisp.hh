#pragma once

#include<list>
#include<set>
#include"myutils/mymap.hh"
#include"value.hh"

namespace zlt::mylisp {
  extern mymap::Node<Value, Value> *globalDefs;
  extern std::set<std::string> strings;

  const std::string *addString(std::string &&s);
  void exec();

  namespace bad {}
}
