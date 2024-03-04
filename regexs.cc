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

  using Match = match_results<string_view::const_iterator>;

  static int makeGroup(deque<Value> &dest, const Match &m, const Value &str, string_view sv, int i, int end);

  Value natfn_regexec(const Value *it, const Value *end) {
    RegexObj *ro;
    string_view sv;
    if (!dynamicasts(make_tuple(&ro, &sv), it, end)) [[unlikely]] {
      return Null();
    }
    Match m;
    if (!regex_search(sv.begin(), sv.end(), m, ro->regex)) {
      return neobj<ListObj>();
    }
    deque<Value> ls(m.size());
    makeGroup(ls, m, it[1], sv, 0, m.size());
    return neobj<ListObj>(std::move(ls));
  }

  int makeGroup(deque<Value> &dest, const Match &m, const Value &str, string_view sv, int i, int end) {
    if (i == end) [[unlikely]] {
      return 0;
    }
    deque<Value> a(3);
    int start = m.position(i);
    int len = m.length(i);
    a[0] = start;
    a[1] = start + len;
    a[2] = Value(str, sv.substr(start, len));
    dest[i] = neobj<ListObj>(std::move(a));
    return makeGroup(dest, m, str, sv, i + 1, end);
  }
}
