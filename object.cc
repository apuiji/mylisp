#include<algorithm>
#include<cmath>
#include"gc.hh"
#include"object.hh"

using namespace std;

namespace zlt::mylisp {
  int FunctionObj::graySubjs() noexcept {
    for (auto &p : closures) {
      gc::grayValue(p.second);
    }
    return 0;
  }

  Value ListObj::objGetMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    if (!(i >= 0 && i < list.size())) {
      return Null();
    }
    return list[i];
  }

  int ListObj::objSetMemb(const Value &memb, const Value &value) {
    int i;
    if (!dynamicast(i, memb)) {
      return 0;
    }
    if (!(i >= 0 && i < list.size())) {
      return 0;
    }
    list[i] = value;
    gc::iwb(this, value);
    return 0;
  }

  int ListObj::graySubjs() noexcept {
    for_each(list.begin(), list.end(), gc::grayValue);
    return 0;
  }

  template<class K, class Comp>
  static Value poolGetMemb(const map<K, Value, Comp> &m, const K &k) noexcept {
    auto it = m.find(k);
    if (it != m.end()) {
      return it->second;
    } else {
      return Null();
    }
  }

  static Value strPoolGetMemb(const MapObj::StrPool &sp, std::wstring_view sv) noexcept {
    auto node = find(sp, sv);
    if (node) {
      return node->second;
    } else {
      return Null();
    }
  }

  Value MapObj::objGetMemb(const Value &memb) const noexcept {
    switch (memb.index()) {
      case Value::NULL_INDEX: {
        if (nullPool.first) {
          return nullPool.second;
        } else {
          return Null();
        }
      }
      case Value::NUM_INDEX: {
        double d;
        staticast(d, memb);
        if (isnan(d)) {
          if (nanPool.first) {
            return nanPool.second;
          } else {
            return Null();
          }
        } else {
          return poolGetMemb(numPool, d);
        }
      }
      case Value::CHAR_INDEX: {
        wstring_view sv;
        staticast<Value::CHAR_INDEX>(sv, memb);
        return strPoolGetMemb(strPool, sv);
      }
      case Value::STR_INDEX: {
        wstring_view sv;
        staticast<Value::STR_INDEX>(sv, memb);
        return strPoolGetMemb(strPool, sv);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, memb);
        if (wstring_view sv; o->objDynamicast(sv)) {
          return strPoolGetMemb(strPool, sv);
        }
        return poolGetMemb(objPool, o);
      }
      default: {
        void *p;
        staticast(p, memb);
        return poolGetMemb(ptrPool, p);
      }
    }
  }

  static Value &mapGetMemb4set(MapObj *m, const Value &memb);

  int MapObj::objSetMemb(const Value &memb, const Value &value) {
    mapGetMemb4set(this, memb) = value;
    gc::iwb(this, value);
    return 0;
  }

  Value &mapGetMemb4set(MapObj *m, const Value &memb) {
    switch (memb.index()) {
      case Value::NULL_INDEX: {
        m->nullPool.first = true;
        return m->nullPool.second;
      }
      case Value::NUM_INDEX: {
        double d;
        staticast(d, memb);
        return isnan(d) ? m->nanPool.second : m->numPool[d];
      }
      case Value::CHAR_INDEX: {
        wstring_view sv;
        staticast<Value::CHAR_INDEX>(sv, memb);
        return insert(m->strPool, sv, memb)->second;
      }
      case Value::STR_INDEX: {
        wstring_view sv;
        staticast<Value::STR_INDEX>(sv, memb);
        return insert(m->strPool, sv, memb)->second;
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, memb);
        gc::iwb(m, o);
        if (wstring_view sv; o->objDynamicast(sv)) {
          return insert(m->strPool, sv, memb)->second;
        }
        return m->objPool[o];
      }
      default: {
        void *p;
        staticast(p, memb);
        return m->ptrPool[p];
      }
    }
  }

  int MapObj::graySubjs() noexcept {
    if (nullPool.first) {
      gc::grayValue(nullPool.second);
    }
    if (nanPool.first) {
      gc::grayValue(nanPool.second);
    }
    for (auto &p : numPool) {
      gc::grayValue(p.second);
    }
    for (auto &p : strPool) {
      gc::grayValue(p.first);
      gc::grayValue(p.second);
    }
    for (auto &p : objPool) {
      gc::grayObj(p.first);
      gc::grayValue(p.second);
    }
    for (auto &p : ptrPool) {
      gc::grayValue(p.second);
    }
    return 0;
  }

  int PointerObj::graySubjs() noexcept {
    gc::grayValue(value);
    return 0;
  }
}
