#include<regex>
#include"ast.hh"
#include"ast_token.hh"

using namespace std;

namespace zlt::mylisp::ast::token {
  static bool isNumber(double &dest, const char *start, std::string_view raw);

  int ofRaw(double &numval, const char *start, std::string_view raw) {
    if (isNumber(numval, start, raw)) {
      return NUMBER;
    }
    #define ifSymbol(symb) \
    if (raw == symb) { \
      return symb##_token; \
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
    return ID;
  }

  static bool isBaseInt(double &dest, string_view src);

  bool isNumber(double &dest, const char *start, string_view src) {
    try {
      if (isBaseInt(dest, src)) {
        return true;
      }
      try {
        dest = stod(string(src));
        return true;
      } catch (invalid_argument) {
        return false;
      }
    } catch (out_of_range) {
      throw AstBad(bad::NUMBER_LITERAL_OOR, start);
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
    dest = stoi(m.str(1) + m.str(2), nullptr, base);
    return true;
  }
}
