#include"regexs.hh"

using namespace std;

namespace zlt::mylisp {
  Value natFnRegcomp(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    wstring_view sv;
    if (!dynamicast(sv, *it)) {
      return Null();
    }
    wstring_view sv1;
    dynamicast(sv1, it[1]);
    auto flags = regex_constants::ECMAScript;
    if (find(sv1.begin(), sv1.end(), 'i') != sv1.end()) {
      flags = flags | regex_constants::icase;
    }
    try {
      return gc::neobj(new RegexObj(wregex(sv, flags)));
    } catch (...) {
      return Null();
    }
  }

  Value natFnRegexec(const Value *it, const Value *end) {
    if (it == end) [[unlikely]] {
      return Null();
    }
    RegexObj *ro;
    if (!dynamicast(ro, *it)) {
      return Null();
    }
    wstring_view sv;
    if (!(it + 1 != end && dynamicast(sv, it[1]))) {
      return Null();
    }
    using Match = match_results<wstring_view::iterator>;
    Match m;
    if (!regex_search(sv, m, ro->regex)) {
      return gc::neobj(new ListObj);
    }
    size_t n = m.size();
    deque<Value> ls(n);
    for (int i = 0; i < n; ++i) {
      deque<Value> a(3);
      int start = m.position(i);
      int len = m.length(i);
      a[0] = start;
      a[1] = start + len;
      a[2] = gc::neobj(new StringViewObj(it[1], sv.substr(start, len)));
    }
  }
}
