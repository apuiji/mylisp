#include<algorithm>
#include<cmath>
#include"gc.hh"
#include"object.hh"

using namespace std;

namespace zlt::mylisp {
  int FunctionObj::graySubjs() noexcept {
    for (auto &p : closures) {
      gc::grayIt(p.second);
    }
    return 0;
  }

  Value ListObj::getMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    if (!(i >= 0 && i < list.size())) {
      return Null();
    }
    return list[i];
  }

  int ListObj::setMemb(const Value &memb, const Value &value) {
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
    for_each(list.begin(), list.end(), gc::grayIt);
    return 0;
  }

  bool SetObj::Comparator::operator ()(const Value &a, const Value &b) const noexcept {
    switch (a.index()) {
      case Value::NULL_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            return false;
          }
          default: {
            return true;
          }
        }
      }
      case Value::NUM_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            return false;
          }
          case Value::NUM_INDEX: {
            double x;
            staticast(x, a);
            double y;
            staticast(y, b);
            if (isnan(x)) {
              return !isnan(y);
            }
            return x < y;
          }
          default: {
            return true;
          }
        }
      }
      case Value::OBJ_INDEX: {
        Object *x;
        staticast(x, a);
        if (int diff; x->compare(diff, b)) {
          return diff < 0;
        }
        if (b.index() < Value::OBJ_INDEX) {
          return false;
        }
        void *y;
        staticast(y, b);
        return x < y;
      }
      default: {
        if (b.index() < Value::OBJ_INDEX) {
          return false;
        }
        void *x;
        staticast(x, a);
        void *y;
        staticast(y, b);
        return x < y;
      }
    }
  }

  int SetObj::graySubjs() noexcept {
    for_each(set.begin(), set.end(), gc::grayIt);
    return 0;
  }

  Value setObjElem(const Value &src) {
    switch (src.index()) {
      case Value::CHAR_INDEX: {
        wchar_t c;
        staticast(c, src);
        auto o = new StringViewObj(c);
        gc::neobj(o);
        return o;
      }
      default: {
        if (wstring_view s; dynamicast(s, src)) {
          auto o = new StringViewObj(src, s);
          gc::neobj(o);
          return o;
        }
        if (string_view s; dynamicast(s, src)) {
          auto o = new Latin1ViewObj(src, s);
          gc::neobj(o);
          return o;
        }
      }
    }
  }

  Value MapObj::
}
