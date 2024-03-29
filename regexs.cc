#include"lists.hh"
#include"regexs.hh"

using namespace std;

namespace zlt::mylisp {
  Value natfn_regcomp(const Value *it, const Value *end) {
    string_view sv;
    if (!dynamicast(sv, it, end)) [[unlikely]] {
      return Null();
    }
    string_view sv1;
    dynamicast(sv1, it + 1, end);
    auto flags = regex_constants::ECMAScript;
    if (find(sv1.begin(), sv1.end(), 'i') != sv1.end()) {
      flags = flags | regex_constants::icase;
    }
    try {
      return neobj<RegexObj>(regex(sv.begin(), sv.end(), flags));
    } catch (...) {
      return Null();
    }
  }

  Value natfn_regexec(const Value *it, const Value *end) {
    RegexObj *ro;
    string_view sv;
    if (!dynamicasts(make_tuple(&ro, &sv), it, end)) [[unlikely]] {
      return Null();
    }
    using Match = match_results<string_view::const_iterator>;
    Match m;
    if (!regex_search(sv.begin(), sv.end(), m, ro->regex)) {
      return neobj<ListObj>();
    }
    size_t n = m.size();
    deque<Value> ls(n);
    for (int i = 0; i < n; ++i) {
      deque<Value> a(3);
      int start = m.position(i);
      int len = m.length(i);
      a[0] = start;
      a[1] = start + len;
      a[2] = Value(it[1], sv.substr(start, len));
      ls[i] = neobj<ListObj>(std::move(a));
    }
    return neobj<ListObj>(std::move(ls));
  }
}
