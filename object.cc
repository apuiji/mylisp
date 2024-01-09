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

  static bool compare1(bool &dest, const Value &a, const Value &b) noexcept;

  bool SetObj::Comparator::operator ()(const Value &a, const Value &b) const noexcept {
    bool bu;
    if (compare1(bu, a, b)) {
      return bu;
    }
    compare1(bu, b, a);
    return !bu;
  }

  bool compare1(bool &dest, const Value &a, const Value &b) noexcept {
    switch (a.index()) {
      case Value::NULL_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            dest = false;
            return true;
          }
          case Value::OBJ_INDEX: {
            return false;
          }
          default: {
            dest = true;
            return true;
          }
        }
      }
      case Value::NUM_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            dest = false;
            return true;
          }
          case Value::NUM_INDEX: {
            double d;
            staticast(d, a);
            double d1;
            staticast(d1, b);
            if (isnan(d)) {
              dest = !isnan(d1)
            } else {
              dest = d < d1;
            }
            return true;
          }
          case Value::OBJ_INDEX: {
            return false;
          }
          default: {
            dest = true;
            return true;
          }
        }
      }
      case Value::STR_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX:
            [[fallthrough]];
          case Value::NUM_INDEX: {
            dest = false;
            return true;
          }
          case Value::STR_INDEX: {
            const wstring *s;
            staticast(s, a);
            const wstring *s1;
            staticast(s1, b);
            dest = *s < *s1;
            return true;
          }
          case Value::OBJ_INDEX: {
            wstring_view sv;
            if (!dynamicast(sv, b)) {
              return false;
            }
            const wstring *s;
            staticast(s, a);
            dest = *s < sv;
            return true;
          }
          default: {
            dest = true;
            return true;
          }
        }
      }
      case Value::LATIN1_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX:
          case Value::NUM_INDEX:
            [[fallthrough]];
          case Value::STR_INDEX: {
            dest = false;
            return true;
          }
          case Value::LATIN1_INDEX: {
            const string *s;
            staticast(s, a);
            const string *s1;
            staticast(s1, b);
            dest = *s < *s1;
            return true;
          }
          case Value::OBJ_INDEX: {
            string_view sv;
            if (!dynamicast(sv, b)) {
              return false;
            }
            const string *s;
            staticast(s, a);
            dest = *s < sv;
            return true;
          }
          default: {
            dest = true;
            return true;
          }
        }
      }
      case Value::NAT_FN_INDEX: {
        switch (b.index()) {
          case Value::NAT_FN_INDEX: {
            NativeFunction *f;
            staticast(f, a);
            NativeFunction *g;
            staticast(g, b);
            dest = f < g;
            return true;
          }
          case Value::OBJ_INDEX: {
            return false;
          }
          default: {
            dest = false;
            return true;
          }
        }
      }
      default: {
        Object *o;
        staticast(o, a);
      }
    }
  }
}
