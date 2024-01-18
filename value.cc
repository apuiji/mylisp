#include"gc.hh"
#include"object.hh"
#include"value.hh"

using namespace std;

namespace zlt::mylisp {
  Value &Value::operator =(wstring &&s) {
    if (s.size() == 1) {
      return operator =(s[0]);
    }
    Object *o = gc::neobj(new StringObj(std::move(s)));
    return operator =(o);
  }

  // cast operations begin
  Value toStringValue(const Value &src) {
    wstring_view sv;
    if (!dynamicast(sv, src)) {
      return L"";
    }
    if (sv.size() == 1) {
      return sv[0];
    }
    return gc::neobj(new StringViewObj(src, sv));
  }

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
    if (Object *o; dynamicast(o, a)) {
      return *o == b;
    }
    if (Object *o; dynamicast(o, b)) {
      return a == *o;
    }
    if (a.index() != b.index()) {
      return false;
    }
    switch (a.index()) {
      case Value::NULL_INDEX: {
        return true;
      }
      #define caseIT(I, T) \
      case Value::I##_INDEX: { \
        T x; \
        staticast(x, a); \
        T y; \
        staticast(y, b); \
        return x == y; \
      }
      caseIT(NUM, double);
      caseIT(CHAR, wchar_t);
      caseIT(STR, const wstring *);
      caseIT(NAT_FN, NativeFunction *);
      #undef caseIT
      default: {
        // never
        return false;
      }
    }
  }

  bool compare(int &dest, const Value &a, const Value &b) noexcept {
    if (Object *o; dynamicast(o, a)) {
      return o->compare(dest, b);
    }
    if (Object *o; dynamicast(o, b)) {
      return compare(dest, a, *o);
    }
    if (a.index() != b.index()) {
      return false;
    }
    switch (a.index()) {
      case Value::NULL_INDEX: {
        dest = 0;
        return true;
      }
      case Value::NUM_INDEX: {
        double x;
        staticast(x, a);
        double y;
        staticast(y, b);
        if (x < y) {
          dest = -1;
          return true;
        }
        if (x > y) {
          dest = 1;
          return true;
        }
        if (x == y) {
          dest = 0;
          return true;
        }
        return false;
      }
      case Value::CHAR_INDEX: {
        wchar_t x;
        staticast(x, a);
        wchar_t y;
        staticast(y, b);
        dest = x < y ? -1 : x > y ? 1 : 0;
        return true;
      }
      case Value::STR_INDEX: {
        const wstring *x;
        staticast(x, a);
        const wstring *y;
        staticast(y, b);
        dest = x->compare(*y);
        return true;
      }
      case Value::NAT_FN_INDEX: {
        NativeFunction *x;
        staticast(x, a);
        NativeFunction *y;
        staticast(y, b);
        if (x == y) {
          dest = 0;
          return true;
        }
        return false;
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
    if (!(i >= 0 && i <= s.size())) {
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
