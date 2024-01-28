#include"gc.hh"
#include"object.hh"
#include"value.hh"

using namespace std;

namespace zlt::mylisp {
  Value::Value(const Value &string, std::wstring_view view) {
    switch (view.size()) {
      case 0: {
        operator =(constring<>);
        break;
      }
      case 1: {
        operator =(view[0]);
        break;
      }
      default: {
        operator =(gc::neobj(new StringViewObj(string, view)));
      }
    }
  }

  Value &Value::operator =(wstring_view sv) {
    switch (sv.size()) {
      case 0: {
        return operator =(constring<>);
      }
      case 1: {
        return operator =(sv[0]);
      }
      default: {
        return operator =(gc::neobj(new StringObj(wstring(sv))));
      }
    }
  }

  // cast operations begin
  bool dynamicast(string_view &dest, const Value &src) noexcept {
    switch (src.index()) {
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, src);
        return o->objDynamicast(dest);
      }
      default: {
        return false;
      }
    }
  }

  bool dynamicast(wstring_view &dest, const Value &src) noexcept {
    switch (src.index()) {
      case Value::CHAR_INDEX: {
        dest = wstring_view((const wchar_t *) &src, 1);
        return true;
      }
      case Value::STR_INDEX: {
        const wstring *s;
        staticast(s, src);
        dest = (wstring_view) *s;
        return true;
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, src);
        return o->objDynamicast(dest);
      }
      default: {
        return false;
      }
    }
  }
  // cast operations end

  // comparisons begin
  bool operator ==(const Value &a, const Value &b) noexcept {
    if (Object *o; dynamicast(o, b)) {
      return a == *o;
    }
    switch (a.index()) {
      case Value::NULL_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            return true;
          }
          default: {
            return false;
          }
        }
      }
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        switch (b.index()) {
          case Value::NUM_INDEX: {
            double y;
            staticast(y, b);
            return x == y;
          }
          default: {
            return false;
          }
        }
      }
      case Value::CHAR_INDEX:
        [[fallthrough]];
      case Value::STR_INDEX: {
        wstring_view x;
        dynamicast(x, a);
        switch (b.index()) {
          case Value::CHAR_INDEX:
            [[fallthrough]];
          case Value::STR_INDEX: {
            wstring_view y;
            dynamicast(y, b);
            return x == y;
          }
          default: {
            return false;
          }
        }
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, a);
        return *o == b;
      }
      default: {
        void *x;
        staticast(x, a);
        switch (b.index()) {
          case Value::NAT_FN_INDEX: {
            void *y;
            staticast(y, b);
            return x == y;
          }
          default: {
            return false;
          }
        }
      }
    }
  }

  bool compare(int &dest, const Value &a, const Value &b) noexcept {
    if (Object *o; dynamicast(o, b)) {
      return compare(dest, a, *o);
    }
    switch (a.index()) {
      case Value::NULL_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            dest = 0;
            return true;
          }
          default: {
            return false;
          }
        }
      }
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        switch (b.index()) {
          case Value::NUM_INDEX: {
            double y;
            staticast(y, b);
            return Compare {}(dest, x, y);
          }
          default: {
            return false;
          }
        }
      }
      case Value::CHAR_INDEX:
        [[fallthrough]];
      case Value::STR_INDEX: {
        wstring_view x;
        dynamicast(x, a);
        switch (b.index()) {
          case Value::CHAR_INDEX:
            [[fallthrough]];
          case Value::STR_INDEX: {
            wstring_view y;
            dynamicast(y, b);
            dest = x.compare(y);
            return true;
          }
          default: {
            return false;
          }
        }
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, a);
        return o->compare(dest, b);
      }
      default: {
        // never
        return false;
      }
    }
  }
  // comparisons end

  template<class C>
  static Value getMemb(basic_string_view<C> s, const Value &memb);

  Value getMemb(const Value &v, const Value &memb) noexcept {
    switch (v.index()) {
      case Value::CHAR_INDEX: {
        return getMemb(wstring_view((const wchar_t *) &v, 1), memb);
      }
      case Value::STR_INDEX: {
        const wstring *s;
        staticast(s, v);
        return getMemb((wstring_view) *s, memb);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, v);
        return o->objGetMemb(memb);
      }
      default: {
        return Null();
      }
    }
  }

  template<class C>
  Value getMemb(basic_string_view<C> s, const Value &memb) {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    if (!(i >= 0 && i < s.size())) {
      return Null();
    }
    return (wchar_t) s[i];
  }

  int setMemb(Value &v, const Value &memb, const Value &value) {
    if (Object *o; dynamicast(o, v)) {
      return o->objSetMemb(memb, value);
    }
    return 0;
  }
}
