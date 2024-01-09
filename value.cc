#include"gc.hh"
#include"object.hh"
#include"value.hh"

using namespace std;

namespace zlt::mylisp {
  static bool equals(bool &dest, const Value &a, const Value &b) noexcept;

  bool operator ==(const Value &a, const Value &b) noexcept {
    bool bu;
    equals(bu, a, b) || equals(bu, b, a);
    return bu;
  }

  static bool equals(bool &dest, const Value &a, const Value &b) noexcept {
    switch (a.index()) {
      case Value::NULL_INDEX: {
        switch (b.index()) {
          case Value::NULL_INDEX: {
            dest = true;
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
      #define unlessObj(I, T) \
      case Value::I: { \
        switch (b.index()) { \
          case Value::I: { \
            T x; \
            staticast(x, a); \
            T y; \
            staticast(y, b); \
            dest = x == y; \
            return true; \
          } \
          case Value::OBJ_INDEX: { \
            return false; \
          } \
          default: { \
            dest = false; \
            return true; \
          } \
        } \
      }
      unlessObj(NUM_INDEX, double);
      unlessObj(STR_INDEX, const wstring *);
      unlessObj(LATIN1_INDEX, const string *);
      unlessObj(NAT_FN_INDEX, NativeFunction *);
      #undef unlessObj
      default: {
        Object *o;
        staticast(o, a);
        dest = *o == b;
        return true;
      }
    }
  }

  template<class C>
  static Value getMemb(const Value &v, basic_string_view<C> s, const Value &memb);

  template<class C>
  static inline Value getMemb(const Value &v, const basic_string<C> &s, const Value &memb) {
    return getMemb(v, (basic_string_view<C>) s, memb);
  }

  Value getMemb(const Value &v, const Value &memb) noexcept {
    switch (v.index()) {
      case Value::STR_INDEX: {
        const wstring *s;
        staticast(s, v);
        return getMemb(v, *s, memb);
      }
      case Value::LATIN1_INDEX: {
        const string *s;
        staticast(s, v);
        return getMemb(v, *s, memb);
      }
      case Value::OBJ_INDEX: {
        Object *o;
        staticast(o, v);
        return o->getMemb(memb);
      }
      default: {
        return Null();
      }
    }
  }

  template<class C>
  Value getMemb(const Value &v, basic_string_view<C> s, const Value &memb) {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    if (!(i >= 0 && i <= s.size())) {
      return Null();
    }
    auto svo = new BasicStringViewObj<C>(v, s);
    gc::neobj(svo);
    return svo;
  }
}
