#include"gc.hh"
#include"myccutils/constr.hh"
#include"strings.hh"

using namespace std;

namespace zlt::mylisp {
  Value::Value(const Value &string, std::string_view view) {
    switch (view.size()) {
      case 0: {
        operator =(constString<>);
        break;
      }
      case 1: {
        operator =(view[0]);
        break;
      }
      default: {
        operator =(neobj<StringViewObj>(string, view));
      }
    }
  }

  Value &Value::operator =(string &&s) {
    switch (s.size()) {
      case 0: {
        return operator =(constString<>);
      }
      case 1: {
        return operator =(s[0]);
      }
      default: {
        return operator =(neobj<StringObj>(std::move(s)));
      }
    }
  }

  Value &Value::operator =(string_view sv) {
    switch (sv.size()) {
      case 0: {
        return operator =(constString<>);
      }
      case 1: {
        return operator =(sv[0]);
      }
      default: {
        return operator =(neobj<StringObj>(string(sv)));
      }
    }
  }

  // cast operations begin
  bool dynamicast(string_view &dest, const Value &src) noexcept {
    switch (src.index()) {
      case Value::CHAR_INDEX: {
        staticast<Value::CHAR_INDEX>(dest, src);
        return true;
      }
      case Value::STR_INDEX: {
        staticast<Value::STR_INDEX>(dest, src);
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
    switch (a.index()) {
      case Value::NULL_INDEX: {
        return b.index() == Value::NULL_INDEX;
      }
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        return x == b;
      }
      case Value::CHAR_INDEX: {
        string_view x;
        staticast<Value::CHAR_INDEX>(x, a);
        return x == b;
      }
      case Value::STR_INDEX: {
        string_view x;
        staticast<Value::STR_INDEX>(x, a);
        return x == b;
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
    switch (a.index()) {
      case Value::NULL_INDEX: {
        if (b.index() == Value::NULL_INDEX) {
          dest = 0;
          return true;
        } else {
          return false;
        }
      }
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        return compare(dest, x, b);
      }
      case Value::CHAR_INDEX: {
        string_view x;
        staticast<Value::CHAR_INDEX>(x, a);
        return compare(dest, x, b);
      }
      case Value::STR_INDEX: {
        string_view x;
        staticast<Value::STR_INDEX>(x, a);
        return compare(dest, x, b);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, a);
        return o->compare(dest, b);
      }
      default: {
        void *x;
        staticast(x, a);
        switch (b.index()) {
          case Value::NAT_FN_INDEX: {
            void *y;
            staticast(y, b);
            if (x == y) {
              dest = 0;
              return true;
            } else {
              return false;
            }
          }
          default: {
            return false;
          }
        }
      }
    }
  }

  bool operator ==(const Value &a, double b) noexcept {
    switch (a.index()) {
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        return x == b;
      }
      default: {
        return false;
      }
    }
  }

  bool compare(int &dest, const Value &a, double b) noexcept {
    switch (a.index()) {
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        auto diff = x <=> b;
        if (diff == partial_ordering::less) {
          dest = -1;
          return true;
        }
        if (diff == partial_ordering::equivalent) {
          dest = 0;
          return true;
        }
        if (diff == partial_ordering::greater) {
          dest = 1;
          return true;
        }
        return false;
      }
      default: {
        return false;
      }
    }
  }

  bool operator ==(const Value &a, string_view b) noexcept {
    switch (a.index()) {
      case Value::CHAR_INDEX: {
        string_view x;
        staticast<Value::CHAR_INDEX>(x, a);
        return x == b;
      }
      case Value::STR_INDEX: {
        string_view x;
        staticast<Value::STR_INDEX>(x, a);
        return x == b;
      }
      case Value::OBJ_INDEX: {
        string_view x;
        return dynamicast(x, a) && x == b;
      }
      default: {
        return false;
      }
    }
  }

  bool compare(int &dest, const Value &a, string_view b) noexcept {
    switch (a.index()) {
      case Value::CHAR_INDEX: {
        string_view x;
        staticast<Value::CHAR_INDEX>(x, a);
        dest = x.compare(b);
        return true;
      }
      case Value::STR_INDEX: {
        string_view x;
        staticast<Value::STR_INDEX>(x, a);
        dest = x.compare(b);
        return true;
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, a);
        return o->compare(dest, b);
      }
      default: {
        return false;
      }
    }
  }
  // comparisons end

  static Value getMemb(string_view s, const Value &memb);

  Value getMemb(const Value &v, const Value &memb) noexcept {
    switch (v.index()) {
      case Value::CHAR_INDEX: {
        string_view s;
        staticast<Value::CHAR_INDEX>(s, v);
        return getMemb(s, memb);
      }
      case Value::STR_INDEX: {
        string_view s;
        staticast<Value::STR_INDEX>(s, v);
        return getMemb(s, memb);
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

  Value getMemb(string_view s, const Value &memb) {
    int i;
    if (dynamicast(i, memb) && i >= 0 && i < s.size()) {
      return s[i];
    } else {
      return Null();
    }
  }

  int setMemb(Value &v, const Value &memb, const Value &value) {
    if (Object *o; dynamicast(o, v)) {
      return o->objSetMemb(memb, value);
    }
    return 0;
  }
}
