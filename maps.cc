#include"gc_wb.hh"
#include"maps.hh"

using namespace std;

namespace zlt::mylisp {
  int strPoolComp(string_view a, const Value &b) noexcept {
    string_view s;
    dynamicast(s, b);
    return a.compare(s);
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

  static Value strPoolGetMemb(const MapObj::StrPool *const &sp, std::string_view sv) noexcept {
    auto node = mymap::find(sp, sv, strPoolComp);
    if (node) {
      return node->value.second;
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
        string_view sv;
        staticast<Value::CHAR_INDEX>(sv, memb);
        return strPoolGetMemb(strPool, sv);
      }
      case Value::STR_INDEX: {
        string_view sv;
        staticast<Value::STR_INDEX>(sv, memb);
        return strPoolGetMemb(strPool, sv);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, memb);
        if (string_view sv; o->objDynamicast(sv)) {
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

  static Value &strPoolGetMemb4set(MapObj::StrPool *&sp, const Value &memb, string_view s);

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
        string_view sv;
        staticast<Value::CHAR_INDEX>(sv, memb);
        return strPoolGetMemb4set(m->strPool, memb, sv);
      }
      case Value::STR_INDEX: {
        string_view sv;
        staticast<Value::STR_INDEX>(sv, memb);
        return strPoolGetMemb4set(m->strPool, memb, sv);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, memb);
        gc::iwb(m, o);
        if (string_view sv; o->objDynamicast(sv)) {
          return strPoolGetMemb4set(m->strPool, memb, sv);
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

  Value &strPoolGetMemb4set(MapObj::StrPool *&sp, const Value &memb, string_view s) {
    auto [slot, parent] = mymap::findToInsert(sp, s, strPoolComp);
    if (!*slot) {
      *slot = new MapObj::StrPool(MapObj::StrPool::Value(memb, Null()));
      (**slot).parent = parent;
    }
    return (**slot).value.second;
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
    for (auto &a : rbtree::makeRange(strPool)) {
      gc::grayValue(a.value.first);
      gc::grayValue(a.value.second);
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
}
