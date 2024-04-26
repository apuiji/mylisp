#include"object.hh"

using namespace std;

namespace zlt::mylisp {
  Value::Value(partial_ordering o) noexcept {
    using O = partial_ordering;
    var = o == O::equivalent ? 0 : o == O::less ? -1 : o == O::greater ? 1 : NAN;
  }

  Value &Value::operator =(partial_ordering o) noexcept {
    using O = partial_ordering;
    var = o == O::equivalent ? 0 : o == O::less ? -1 : o == O::greater ? 1 : NAN;
    return *this;
  }

  bool dynamicast(string_view &dest, const Value &value) noexcept {
    if (value.var.index() == Value::STR_INDEX) {
      dest = string_view(*staticast<const string *>(value));
      return true;
    }
    if (value.var.index() == Value::OBJ_INDEX) {
      auto o = staticast<Object *>(value);
      auto s = dynamic_cast<V2StringViewObj *>(o);
      dest = s->toStringView();
      return true;
    }
    return false;
  }

  partial_ordering operator <=>(const Value &a, const Value &b) noexcept {
    using O = partial_ordering;
    size_t i = a.var.index();
    size_t j = b.var.index();
    if (i == Value::NULL_INDEX) {
      return j == Value::NULL_INDEX ? O::equivalent : O::unordered;
    }
    if (i == Value::NUM_INDEX) {
      return j == Value::NUM_INDEX ? staticast<double>(a) <=> staticast<double>(b) : O::unordered;
    }
    if (i == Value::STR_INDEX) {
      string_view s;
      if (!dynamicast(s, b)) {
        return O::unordered;
      }
      auto s1 = (string_view) *staticast<const string *>(a);
      return s1.compare(s) <=> 0;
    }
    if (i == Value::OBJ_INDEX) {
      return staticast<Object *>(a)->compare(b);
    }
    return i == j && staticast<void *>(a) == staticast<void *>(b) ? O::equivalent : O::unordered;
  }

  bool length(size_t &dest, const Value &src) noexcept {
    if (const string *s; dynamicast(s, src)) {
      dest = s->size();
      return true;
    }
    if (Object *o; dynamicast(o, src)) {
      return o->length(dest);
    }
    return false;
  }

  Value getMemb(const Value &cont, const Value &key) noexcept {
    if (const string *s; dynamicast(s, cont)) {
      if (int i; dynamicast(i, key) && i >= 0 && i < s->size()) {
        return (*s)[i];
      }
      return Null();
    }
    if (Object *o; dynamicast(o, cont)) {
      return o->getMemb(key);
    }
    return Null();
  }

  void setMemb(Value &cont, const Value &key, const Value &value) {
    if (Object *o; dynamicast(o, cont)) {
      return o->setMemb(key, value);
    }
  }
}
