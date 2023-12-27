#include<algorithm>
#include<cmath>
#include<cwchar>
#include<cwctype>
#include<regex>
#include<sstream>
#include"ast_lexer.hh"
#include"ast_token.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const wchar_t *;

  template<class C>
  struct LexerStr {
    basic_stringstream<C> &dest;
    int quot;
    It operator ()(It it, It end);
  };

  static bool isRawChar(wchar_t c) noexcept;
  static bool isNumber(double &dest, wstring_view src);
  static bool isToken(uint64_t &dest, wstring_view src) noexcept;

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
      return { token::LPAREN, it, it + 1 };
    }
    if (*it == ')') {
      return { token::RPAREN, it, it + 1 };
    }
    if (!wcsncmp(it, L"latin\"", 6) || !wcsncmp(it, L"latin'", 6)) {
      stringstream ss;
      int quot = it[5];
      It it1 = LexerStr(ss, quot)(it + 6, end);
      if (*it1 != quot) {
        throw LexerBad(it, "unterminated string");
      }
      strval = ss.str();
      if (strval.size() == 1) {
        charval = strval[0];
        return { token::CHAR, it, it1 + 1 };
      } else {
        return { token::STRING, it, it1 + 1 };
      }
    }
    if (*it == '"' || *it == '\'') {
      wstringstream ss;
      It it1 = LexerStr(ss, *it)(it + 1, end);
      if (*it1 != *it) {
        throw LexerBad(it, "unterminated string");
      }
      wstrval = ss.str();
      if (wstrval.size() == 1) {
        charval = wstrval[0];
        return { token::CHAR, it, it1 + 1 };
      } else {
        return { token::WSTRING, it, it1 + 1 };
      }
    }
    size_t n = count_if(it, end, isRawChar);
    if (!n) {
      throw LexerBad(it, "unrecognized symbol");
    }
    raw = wstring_view(it, n);
    if (isNumber(numval, raw)) {
      return { token::NUMBER, it, it + n };
    }
    if (uint64_t t; isToken(t, raw)) {
      return { t, it, it + n };
    }
    return { token::ID, it, it + n };
  }

  struct StrHit {
    int quot;
    bool operator ()(wchar_t c) noexcept;
  };

  static bool esch(int &dest, size_t &len, It it, It end);

  template<class C>
  It LexerStr::operator ()(It it, It end) {
    if (it == end) [[unlikely]] {
      return end;
    }
    if (It it1 = find_if(it, end, StrHit(quot)); it1 != it) {
      dest.write(it, it1 - it);
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

  bool StrHit::operator ()(wchar_t c) noexcept {
    return c == '\\' || c == quot;
  }

  static bool esch1(int &dest, size_t &len, It it, It end) noexcept;
  static bool esch8(int &dest, size_t &len, It it, It end);
  static bool eschx(int &dest, size_t &len, It it, It end) noexcept;
  static bool eschu(int &dest, size_t &len, It it, It end) noexcept;

  bool esch(int &dest, size_t &len, It it, It end) {
    return esch1(dest, len, it, end) || esch8(dest, len, it, end) || eschx(dest, len, it, end) || eschu(dest, len, it, end);
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
    static const wregex re(L"^[0-3][0-7]{0,2}|^[4-7][0-7]?");
    wcmatch m;
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

  bool eschu(int &dest, size_t &len, It it, It end) noexcept {
    if (*it == 'u' && all_of(it + 1, it + 5, isxdigit)) {
      dest = (it[1] << 12) | (it[2] << 8) | (it[3] << 4) | it[4];
      len = 5;
      return true;
    } else {
      return false;
    }
  }

  bool isRawChar(wchar_t c) noexcept {
    return !strchr("\"'();") && !iswspace(c);
  }

  static bool isBaseInt(double &dest, wstring_view src)

  bool isNumber(double &dest, wstring_view src) {
    if (isBaseInt(dest, src)) {
      return true;
    } else {
      dest = stod(src);
      return isnan(dest);
    }
  }

  static bool isBaseInt(double &dest, const wregex &re, size_t base, wstring_view src);

  bool isBaseInt(double &dest, wstring_view src) {
    static const wregex re2(L"([+-]?)0[Bb]([01]+)");
    static const wregex re4(L"([+-]?)0[Qq]([0-3]+)");
    static const wregex re8(L"([+-]?)0[Oo]([0-7]+)");
    static const wregex re16(L"([+-]?)0[Xx]([[:xdigit:]]+)");
    return
      isBaseInt(dest, re2, 2, src) ||
      isBaseInt(dest, re4, 4, src) ||
      isBaseInt(dest, re8, 8, src) ||
      isBaseInt(dest, re16, 16, src);
  }

  bool isBaseInt(double &dest, const wregex &re, size_t base, wstring_view src) {
    match_results<wstring_view::const_iterator> m;
    if (!regex_match(src, m, re)) {
      return false;
    }
    dest = stoi(m.str(1) + m.str(2), nullptr, base);
    return true;
  }

  bool isToken(uint64_t &dest, wstring_view raw) noexcept {
    #define ifKeyword(kwd) \
    if (raw == L###kwd) { \
      dest = token::KWD_##kwd; \
      return true; \
    }
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
    #define ifPreprocDir(dir) \
    if (raw == L"#" #dir) { \
      dest = token::DIR_##dir; \
      return true; \
    }
    ifPreprocDir(def);
    ifPreprocDir(ifndef);
    ifPreprocDir(include);
    ifPreprocDir(undef);
    #undef ifPreprocDir
    #define ifSymbol(symb) \
    if (raw == L##symb) { \
      dest = token::symbol(symb); \
      return true; \
    }
    ifSymbol("!");
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
