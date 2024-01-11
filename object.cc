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
    return 0;
  }

  int ListObj::graySubjs() noexcept {
    for_each(list.begin(), list.end(), gc::grayValue);
    return 0;
  }

  template<class K>
  static Value poolGetMemb(const map<K, Value> &m, const K &k) noexcept {
    auto it = m.find(k);
    if (it != m.end()) {
      return it->second;
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
        wchar_t c;
        staticast(c, memb);
        return poolGetMemb(charPool, c);
      }
      case Value::STR_INDEX: {
        const wstring *s;
        staticast(s, memb);
        BasicStringViewKey<wchar_t> k(s, (wstring_view) *s);
        return poolGetMemb(strPool, k);
      }
      case Value::LATIN1_INDEX: {
        const string *s;
        staticast(s, memb);
        BasicStringViewKey<char> k(s, (string_view) *s);
        return poolGetMemb(latin1Pool, k);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, memb);
        if (auto s = dynamic_cast<StringViewObj *>(o); s) {
          BasicStringViewKey<wchar_t> k(o, (wstring_view) *s);
          return poolGetMemb(strPool, k);
        }
        if (auto s = dynamic_cast<Latin1ViewObj *>(o); s) {
          BasicStringViewKey<char> k(o, (string_view) *s);
          return poolGetMemb(latin1Pool, k);
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
        wchar_t c;
        staticast(c, memb);
        return m->charPool[c];
      }
      case Value::STR_INDEX: {
        const wstring *s;
        staticast(s, memb);
        MapObj::BasicStringViewKey<wchar_t> k(s, (wstring_view) *s);
        return m->strPool[k];
      }
      case Value::LATIN1_INDEX: {
        const string *s;
        staticast(s, memb);
        MapObj::BasicStringViewKey<char> k(s, (string_view) *s);
        return m->latin1Pool[k];
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, memb);
        if (auto s = dynamic_cast<StringViewObj *>(o); s) {
          MapObj::BasicStringViewKey<wchar_t> k(o, (wstring_view) *s);
          return m->strPool[k];
        }
        if (auto s = dynamic_cast<Latin1ViewObj *>(o); s) {
          MapObj::BasicStringViewKey<char> k(o, (string_view) *s);
          return m->latin1Pool[k];
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
    for (auto &p : charPool) {
      gc::grayValue(p.second);
    }
    for (auto &p : strPool) {
      gc::grayValue(p.first.string);
      gc::grayValue(p.second);
    }
    for (auto &p : latin1Pool) {
      gc::grayValue(p.first.string);
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
