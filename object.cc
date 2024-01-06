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

  int ListObj::graySubjs() noexcept {
    for_each(list.begin(), list.end(), gc::grayIt);
    return 0;
  }

  int SetObj::graySubjs() noexcept {
    for (auto &v : set) {
      gc::grayIt(const_cast<Value &>(v));
    }
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
            double d;
            double d1;
            staticast(d, a);
            staticast(d1, b);
            if (isnan(d)) {
              return !isnan(d1);
            }
            return d < d1;
          }
          default: {
            return true;
          }
        }
      }
      case Value::STR_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX:
            [[fallthrough]];
          case Value::NUM_INDEX: {
            return false;
          }
          default: {
            const wstring *s;
            wstring_view s1;
            staticast(s, a);
            if (!dynamicast(s1, b)) {
              return true;
            }
            return *s < s1;
          }
        }
      }
      case Value::LATIN1_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX:
          case Value::NUM_INDEX:
            [[fallthrough]];
          case Value::STR_INDEX: {
            return false;
          }
          default: {
            const string *s;
            string_view s1;
            staticast(s, a);
            if (!dynamicast(s1, b)) {
              return true;
            }
            return *s < s1;
          }
        }
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, a);
        if (int diff; o->compare(diff, b) && diff < 0) {
          return true;
        }
        [[fallthrough]];
      }
      default: {
        switch (b.index()) {
          case Value::NULL_INDEX:
          case Value::NUM_INDEX:
          case Value::STR_INDEX:
            [[unlikely]];
          case Value::LATIN1_INDEX: {
            return false;
          }
          default: {
            void *p;
            void *q;
            staticast(p, a);
            staticast(q, b);
            return p < q;
          }
        }
      }
    }
  }
}
