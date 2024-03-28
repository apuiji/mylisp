#include<algorithm>
#include<cctype>
#include<cstring>
#include<regex>
#include<sstream>
#include"ast.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const char *;

  It hit(It it, It end) noexcept {
    if (it == end) [[unlikely]] {
      return it;
    }
    if (isspace(*it)) {
      return hit(it + 1, end);
    }
    if (*it == ';') {
      return hit(find(it + 1, end, '\n'), end);
    }
    return it;
  }

  struct LexerStr {
    stringstream &dest;
    It start;
    int quot;
    LexerStr(stringstream &dest, It start, int quot) noexcept: dest(dest), start(start), quot(quot) {}
    It operator ()(It it, It end);
  };

  static bool notRawChar(char c) noexcept;
  static bool isNumber(double &dest, string_view src);
  static bool isToken(int &dest, string_view src) noexcept;

  tuple<int, It> Lexer::operator ()(It it, It end) {
    if (*it == '(') {
      return { "("_token, it + 1 };
    }
    if (*it == ')') {
      return { ")"_token, it + 1 };
    }
    if (*it == '"' || *it == '\'') {
      stringstream ss;
      It it1 = LexerStr(ss, *it)(it + 1, end);
      strval = ss.str();
      int t;
      if (strval.size() == 1) {
        charval = strval[0];
        t = token::CHAR;
      } else {
        t = token::STRING;
      }
      return { t, it1 };
    }
    It it1 = find_if(it, end, notRawChar);
    if (it1 == it) {
      throw AstBad(bad::UNRECOGNIZED_SYMBOL, it);
    }
    raw = string_view(it, it1);
    try {
      if (isNumber(numval, raw)) {
        return { token::NUMBER, it1 };
      }
    } catch (out_of_range) {
      throw AstBad(bad::NUMBER_LITERAL_OOR, it);
    }
    if (int t; isToken(t, raw)) {
      return { t, it1 };
    }
    return { token::ID, it1 };
  }

  static pair<int, size_t> esch(It it, It end);

  It LexerStr::operator ()(It it, It end) {
    if (it == end) [[unlikely]] {
      throw AstBad(bad::UNTERMINATED_STRING, start);
    }
    if (It it1 = find_if(it, end, [quot = this->quot] (auto c) { return c == '\\' || c == quot; }); it1 != it) {
      dest.write(it, it1 - it);
      return operator ()(it1, end);
    }
    if (*it == '\\') {
      auto [c, n] = esch(it + 1, end);
      dest.put(c);
      return operator ()(it + 1 + n, end);
    }
    return it;
  }

  static bool esch1(int &dest, size_t &len, It it, It end) noexcept;
  static bool esch8(int &dest, size_t &len, It it, It end);
  static bool eschx(int &dest, size_t &len, It it, It end) noexcept;

  pair<int, size_t> esch(int &dest, size_t &len, It it, It end) {
    int c;
    size_t n;
    esch1(c, n, it, end) || esch8(c, n, it, end) || eschx(c, n, it, end) || (c = '\\', n = 0);
    return { c, n };
  }

  bool esch1(int &dest, size_t &len, It it, It end) noexcept {
    if (*it == '"' || *it == '\'' || *it == '\\') {
      dest = *it;
    } else if (*it == 'n') {
      dest = '\n';
    } else if (*it == 'r') {
      dest = '\r';
    } else if (*it == 't') {
      dest = '\t';
    } else {
      return false;
    }
    len = 1;
    return true;
  }

  bool esch8(int &dest, size_t &len, It it, It end) {
    static const regex re("^[0-3][0-7]{0,2}|^[4-7][0-7]?");
    cmatch m;
    if (!regex_search(it, end, m, re)) {
      return false;
    }
    dest = stoi(m.str(), nullptr, 8);
    len = m.length();
    return true;
  }

  bool eschx(int &dest, size_t &len, It it, It end) noexcept {
    if (*it == 'x' && it + 2 < end && isxdigit(it[1]) && isxdigit(it[2])) {
      dest = (it[1] << 4) | it[2];
      len = 3;
      return true;
    } else {
      return false;
    }
  }

  bool notRawChar(char c) noexcept {
    return strchr("\"'();", c) || isspace(c);
  }

  static bool isBaseInt(double &dest, string_view src);

  bool isNumber(double &dest, string_view src) {
    if (isBaseInt(dest, src)) {
      return true;
    }
    try {
      dest = stod(string(src));
      return true;
    } catch (invalid_argument) {
      return false;
    }
  }

  static bool isBaseInt(double &dest, const regex &re, size_t base, string_view src);

  bool isBaseInt(double &dest, string_view src) {
    static const regex re2("([+-]?)0[Bb]([01]+)");
    static const regex re4("([+-]?)0[Qq]([0-3]+)");
    static const regex re8("([+-]?)0[Oo]([0-7]+)");
    static const regex re16("([+-]?)0[Xx]([[:xdigit:]]+)");
    return
      isBaseInt(dest, re2, 2, src) ||
      isBaseInt(dest, re4, 4, src) ||
      isBaseInt(dest, re8, 8, src) ||
      isBaseInt(dest, re16, 16, src);
  }

  bool isBaseInt(double &dest, const regex &re, size_t base, string_view src) {
    match_results<string_view::const_iterator> m;
    if (!regex_match(src.begin(), src.end(), m, re)) {
      return false;
    }
    try {
      dest = stoi(m.str(1) + m.str(2), nullptr, base);
      return true;
    } catch (invalid_argument) {
      return false;
    }
  }

  bool isToken(int &dest, string_view raw) noexcept {
    #define ifSymbol(symb) \
    if (raw == symb) { \
      dest = symb##_token; \
      return true; \
    }
    #define ifKeyword(kwd) ifSymbol(#kwd)
    ifKeyword(callee);
    ifKeyword(def);
    ifKeyword(defer);
    ifKeyword(forward);
    ifKeyword(if);
    ifKeyword(length);
    ifKeyword(return);
    ifKeyword(throw);
    ifKeyword(try);
    ifKeyword(yield);
    #undef ifKeyword
    ifSymbol("!");
    ifSymbol("#");
    ifSymbol("##");
    ifSymbol("#def");
    ifSymbol("#def");
    ifSymbol("#ifdef");
    ifSymbol("#ifndef");
    ifSymbol("#include");
    ifSymbol("#undef");
    ifSymbol("%");
    ifSymbol("&&");
    ifSymbol("&");
    ifSymbol("**");
    ifSymbol("*");
    ifSymbol("+");
    ifSymbol(",");
    ifSymbol("-");
    ifSymbol(".");
    ifSymbol("/");
    ifSymbol("<<");
    ifSymbol("<=>");
    ifSymbol("<=");
    ifSymbol("<");
    ifSymbol("==");
    ifSymbol("=");
    ifSymbol(">=");
    ifSymbol(">>>");
    ifSymbol(">>");
    ifSymbol(">");
    ifSymbol("@");
    ifSymbol("^^");
    ifSymbol("^");
    ifSymbol("||");
    ifSymbol("|");
    ifSymbol("~");
    #undef ifSymbol
    return false;
  }
}
