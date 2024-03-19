#include<algorithm>
#include<cctype>
#include<cstring>
#include<regex>
#include<sstream>
#include"ast.hh"
#include"ast_lexer.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const char *;

  struct LexerStr {
    stringstream &dest;
    int quot;
    LexerStr(stringstream &dest, int quot) noexcept: dest(dest), quot(quot) {}
    It operator ()(It it, It end);
  };

  static bool notRawChar(char c) noexcept;
  static bool isNumber(double &dest, string_view src);
  static bool isToken(uint64_t &dest, string_view src) noexcept;

  tuple<uint64_t, It, It> Lexer::operator ()(It it, It end) {
    if (it == end) [[unlikely]] {
      return { token::E0F, end, end };
    }
    if (It it1 = find_if_not(it, end, iswspace); it1 != it) {
      return operator ()(it1, end);
    }
    if (*it == ';') {
      It it1 = find(it + 1, end, '\n');
      return operator ()(it1, end);
    }
    if (*it == '(') {
      return { "("_token, it, it + 1 };
    }
    if (*it == ')') {
      return { ")"_token, it, it + 1 };
    }
    if (*it == '"' || *it == '\'') {
      stringstream ss;
      It it1 = LexerStr(ss, *it)(it + 1, end);
      if (*it1 != *it) {
        throw AstBad(bad::UNTERMINATED_STRING, it);
      }
      strval = ss.str();
      if (strval.size() == 1) {
        charval = strval[0];
        return { token::CHAR, it, it1 + 1 };
      }
      return { token::STRING, it, it1 + 1 };
    }
    size_t n = find_if(it, end, notRawChar) - it;
    if (!n) {
      throw AstBad(bad::UNRECOGNIZED_SYMBOL, it);
    }
    raw = string_view(it, n);
    try {
      if (isNumber(numval, raw)) {
        return { token::NUMBER, it, it + n };
      }
    } catch (out_of_range) {
      throw AstBad(bad::NUMBER_LITERAL_OOR, it);
    }
    if (uint64_t t; isToken(t, raw)) {
      return { t, it, it + n };
    }
    return { token::ID, it, it + n };
  }

  static bool esch(int &dest, size_t &len, It it, It end);

  It LexerStr::operator ()(It it, It end) {
    if (it == end) [[unlikely]] {
      return end;
    }
    if (It it1 = find_if(it, end, [quot = this->quot] (auto c) { return c == '\\' || c == quot; }); it1 != it) {
      dest << string(it, it1);
      return operator ()(it1, end);
    }
    if (*it == '\\') {
      int c;
      size_t len;
      if (esch(c, len, it + 1, end)) {
        dest.put(c);
        return operator ()(it + 1 + len, end);
      } else {
        dest.put('\\');
        return operator ()(it + 1, end);
      }
    }
    return it;
  }

  static bool esch1(int &dest, size_t &len, It it, It end) noexcept;
  static bool esch8(int &dest, size_t &len, It it, It end);
  static bool eschx(int &dest, size_t &len, It it, It end) noexcept;

  bool esch(int &dest, size_t &len, It it, It end) {
    return esch1(dest, len, it, end) || esch8(dest, len, it, end) || eschx(dest, len, it, end);
  }

  bool esch1(int &dest, size_t &len, It it, It end) noexcept {
    switch (*it) {
      case '"':
      case '\'':
      case '\\': {
        dest = *it;
        len = 1;
        return true;
      }
      case 'n': {
        dest = '\n';
        len = 1;
        return true;
      }
      case 'r': {
        dest = '\r';
        len = 1;
        return true;
      }
      case 't': {
        dest = '\t';
        len = 1;
        return true;
      }
      default: {
        return false;
      }
    }
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
    if (*it == 'x' && isxdigit(it[1]) && isxdigit(it[2])) {
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

  bool isToken(uint64_t &dest, string_view raw) noexcept {
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
    ifSymbol("#file");
    ifSymbol("#ifdef");
    ifSymbol("#ifndef");
    ifSymbol("#include");
    ifSymbol("#line");
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
