#include<algorithm>
#include"gc.hh"
#include"myutils/xyz.hh"
#include"object.hh"

using namespace std;

namespace zlt::mylisp {
  void *FunctionObj::operator new(size_t size, size_t closureDefn) {
    return ::operator new(size + sizeof(ValueObj *) * closureDefn);
  }

  void FunctionObj::gcMarkSubjs() noexcept {
    ValueObj **p = closureDefs;
    for_each(p, p + closureDefn, OFR<Object *> {}(gc::mark));
  }

  bool ListObj::length(size_t &dest) const noexcept {
    dest = list.size();
    return true;
  }

  Value ListObj::getMemb(const Value &key) const noexcept {
    if (int i; mylisp::dynamicast(i, key) && i >= 0 && i < list.size()) {
      return list[i];
    }
    return Null();
  }

  void ListObj::setMemb(const Value &key, const Value &value) {
    if (int i; mylisp::dynamicast(i, key) && i >= 0 && i < list.size()) {
      list[i] = value;
    }
  }

  void ListObj::gcMarkSubjs() noexcept {
    for_each(list.begin(), list.end(), OFR<Value &> {}(gc::mark));
  }

  Value MapObj::getMemb(const Value &key) const noexcept {
    auto node = mymap::find(map, key);
    if (node) {
      return node->value.second;
    } else {
      return Null();
    }
  }

  void MapObj::setMemb(const Value &key, const Value &value) {
    Node *node;
    if (mymap::insert(node, map, key, [] () { return new Node; })) {
      node->value.first = key;
    }
    node->value.second = value;
  }

  void MapObj::gcMarkSubjs() noexcept {
    for (auto [it, end] = make_pair(mymap::begin(map), mymap::end(map)); it != end; ++it) {
      gc::mark(it->first);
      gc::mark(it->second);
    }
  }

  bool StringObj::length(size_t &dest) const noexcept {
    dest = value.size();
    return true;
  }

  partial_ordering StringObj::compare(const Value &v) const noexcept {
    string_view s;
    return mylisp::dynamicast(s, v) ? value.compare(s) <=> 0 : partial_ordering::unordered;
  }

  bool StringObj::dynamicast(string_view &dest) const noexcept {
    dest = value;
    return true;
  }

  Value StringObj::getMemb(const Value &key) const noexcept {
    if (int i; mylisp::dynamicast(i, key) && i >= 0 && i < value.size()) {
      return value[i];
    }
    return Null();
  }

  bool StringViewObj::length(size_t &dest) const noexcept {
    dest = value.size();
    return true;
  }

  partial_ordering StringViewObj::compare(const Value &v) const noexcept {
    string_view s;
    return mylisp::dynamicast(s, v) ? value.compare(s) <=> 0 : partial_ordering::unordered;
  }

  bool StringViewObj::dynamicast(string_view &dest) const noexcept {
    dest = value;
    return true;
  }

  Value StringViewObj::getMemb(const Value &key) const noexcept {
    if (int i; mylisp::dynamicast(i, key) && i >= 0 && i < value.size()) {
      return value[i];
    }
    return Null();
  }

  void StringViewObj::gcMarkSubjs() noexcept {
    gc::mark(string);
  }

  void ValueObj::gcMarkSubjs() noexcept {
    gc::mark(value);
  }
}
