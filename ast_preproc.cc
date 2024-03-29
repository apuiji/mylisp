#include<algorithm>
#include<cstring>
#include<sstream>
#include"ast_nodes.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = UNodes::const_iterator;

  template<AnyOf<NumberAtom, CharAtom, StringAtom, IDAtom, TokenAtom> T>
  static inline int clone(UNodes &dest, const T &src) {
    dest.push_back(UNode(new T(src)));
    return 0;
  }

  static int clone(UNodes &dest, const List &src) {
    UNodes items;
    clone(items, src.items.begin(), src.items.end());
    UNode a(new List(src.start, std::move(items)));
    dest.push_back(std::move(a));
    return 0;
  }

  static int clone(UNodes &dest, const UNode &src) {
    #define ifType(T) \
    if (auto a = dynamic_cast<const T *>(src.get()); a) { \
      return clone(dest, *a); \
    }
    ifType(NumberAtom);
    ifType(CharAtom);
    ifType(StringAtom);
    ifType(IDAtom);
    ifType(TokenAtom);
    #undef ifType
    return clone(dest, static_cast<const List *>(src.get()));
  }

  static int clone(UNodes &dest, It it, It end);

  static int preprocList(UNodes &dest, Ast &ast, const char *start, It it, It end);

  int preproc(UNodes &dest, Ast &ast, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto ls = dynamic_cast<const List*>(it->get()); ls && ls->items.size()) {
      preprocList(dest, ast, ls->start, ls->items.begin(), ls->items.end());
      return preproc(dest, ast, ++it, end);
    }
    clone(dest, *it);
    return preproc(dest, ast, ++it, end);
  }

  using PreprocDir = int (UNodes &dest, Ast &ast, const char *start, It it, It end);

  static PreprocDir *isPreprocDir(const UNode &src) noexcept;
  static const Macro *findMacro(const Ast &ast, const UNode &src) noexcept;

  struct MacroExpand {
    map<const string *, It> map;
    It endArg;
    int operator ()(UNodes &dest, It it, It end);
  };

  static int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, It it, It end);

  int preprocList(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    if (auto dir = isPreprocDir(*it); dir) {
      return dir(dest, ast, start, ++it, end);
    }
    if (auto m = findMacro(ast, *it); m) {
      UNodes a;
      {
        MacroExpand me;
        makeMacroExpand(me, m->params.begin(), m->params.end(), ++it, end);
        me.endArg = end;
        me(a, m->body.begin(), m->body.end());
      }
      return preproc(dest, ast, a.begin(), a.end());
    }
    UNodes items;
    preproc(items, ast, it, end);
    UNode a(new List(start, std::move(items)));
    dest.push_back(std::move(a));
    return 0;
  }

  template<int T>
  PreprocDir preprocDir;

  #define declPreprocDir(symb) \
  template<> \
  PreprocDir preprocDir<symb##_token>

  declPreprocDir("#");
  declPreprocDir("##");
  declPreprocDir("#def");
  declPreprocDir("#ifdef");
  declPreprocDir("#ifndef");
  declPreprocDir("#include");
  declPreprocDir("#undef");

  #undef declPreprocDir

  PreprocDir *isPreprocDir(const UNode &src) noexcept {
    auto a = dynamic_cast<const TokenAtom *>(src.get());
    if (!a) {
      return nullptr;
    }
    #define ifPreprocDir(symb) \
    if (a->token == symb##_token) { \
      return preprocDir<symb##_token>; \
    }
    ifPreprocDir("#");
    ifPreprocDir("##");
    ifPreprocDir("#def");
    ifPreprocDir("#ifdef");
    ifPreprocDir("#ifndef");
    ifPreprocDir("#include");
    ifPreprocDir("#undef");
    #undef ifPreprocDir
    return nullptr;
  }

  const Macro *findMacro(const Ast &ast, const UNode &src) noexcept {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      return nullptr;
    }
    auto it = ast.macros.find(id->name);
    if (it == ast.macros.end()) {
      return nullptr;
    }
    return &it->second;
  }

  static int makeMacroExpand1(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, It end);

  int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, It it, It end) {
    if (itParam == endParam) [[unlikely]] {
      return 0;
    }
    if (it == end) [[unlikely]] {
      return makeMacroExpand1(dest, itParam, endParam, end);
    }
    if (*itParam) {
      dest.map[*itParam] = it;
    }
    return makeMacroExpand(dest, itParam + 1, endParam, ++it, end);
  }

  int makeMacroExpand1(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, It end) {
    if (itParam == endParam) [[unlikely]] {
      return 0;
    }
    if (*itParam) {
      dest.map[*itParam] = end;
    }
    return makeMacroExpand1(dest, itParam + 1, endParam, end);
  }

  int MacroExpand::operator ()(UNodes &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(it->get()); id) {
      auto it1 = map.find(id->name);
      if (it1 == map.end()) {
        clone(dest, *id);
        return operator ()(dest, ++it, end);
      }
      if (it1->second == endArg) {
        return operator ()(dest, ++it, end);
      }
      if (!strncmp(it1->first->data(), "...", 3)) {
        clone(dest, it1->second, endArg);
        return operator ()(dest, ++it, end);
      }
      clone(dest, *it1->second);
      return operator ()(dest, ++it, end);
    }
    if (auto ls = dynamic_cast<const List *>(it->get()); ls) {
      {
        UNodes items;
        operator ()(items, ls->items.begin(), ls->items.end());
        UNode a(new List(ls->start, std::move(items)));
        dest.push_back(std::move(a));
      }
      return operator ()(dest, ++it, end);
    }
    clone(dest, *it);
    return operator ()(dest, ++it, end);
  }

  template<>
  int preprocDir<"#"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    string s;
    if (it == end) [[unlikely]] {
      s = "";
    } else if (auto a = dynamic_cast<const RawAtom *>(it->get()); a) {
      s = a->raw;
    } else {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, (**it).start);
    }
    auto value = rte::addString(std::move(s));
    UNode a(new StringAtom(start, value));
    dest.push_back(std::move(a));
    return 0;
  }

  static int rawCat(ostream &dest, It it, It end);

  template<>
  int preprocDir<"##"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    stringstream ss;
    rawCat(ss, it, end);
    auto raw = rte::addString(ss.str());
    double d;
    int t = token::ofRaw(d, start, *raw);
    UNode a;
    if (t == token::NUMBER) {
      a.reset(new NumberAtom(start, *raw, d));
    } else if (t == token::ID) {
      a.reset(new IDAtom(start, *raw, raw));
    } else {
      a.reset(new TokenAtom(start, *raw, t));
    }
    dest.push_back(std::move(a));
    return 0;
  }

  int rawCat(ostream &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto a = dynamic_cast<const RawAtom *>(it->get()); a) {
      dest << a->raw;
    } else {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, (**it).start);
    }
    return idcat(dest, ++it, end);
  }

  static int makeMacroParams(Macro::Params &dest, It it, It end);

  template<>
  int preprocDir<"#def"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    if (it == end) {
      return 0;
    }
    auto id = dynamic_cast<const IDAtom *>(it->get());
    if (!id) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, (**it).start);
    }
    if (ast.macros.find(id->name) != ast.macros.end()) {
      throw AstBad(bad::MACRO_ALREADY_EXISTS, start);
    }
    if (++it == end) {
      ast.macros[id->name] = Macro();
      return 0;
    }
    auto ls = dynamic_cast<const List *>(it->get());
    if (!ls) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, (**it).start);
    }
    Macro::Params params;
    makeMacroParams(params, ls->items.begin(), ls->items.end());
    params.shrink_to_fit();
    UNodes body;
    clone(body, ++it, end);
    ast.macros[id->name] = Macro(std::move(params), std::move(body));
    return 0;
  }

  int makeMacroParams(Macro::Params &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(it->get()); id) {
      dest.push_back(id->name);
      if (!strncmp(id->name->data(), "...", 3) && next(it) == end) {
        throw AstBad(bad::MACRO_REST_PARAM_MUST_BE_LAST, id->start);
      }
      return makeMacroParams(dest, ++it, end);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls && ls->items.empty()) {
      dest.push_back(nullptr);
      return makeMacroParams(dest, ++it, end);
    }
    throw AstBad(bad::ILLEGAL_MACRO_PARAM, (**it).start);
  }

  static bool ifdef(const Ast &ast, const UNode &src);

  template<>
  int preprocDir<"#ifdef"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    return it != end && ifdef(ast, *it) ? preproc(dest, ast, ++it, end) : 0;
  }

  template<>
  int preprocDir<"#ifndef"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    return it != end && !ifdef(ast, *it) ? preproc(dest, ast, ++it, end) : 0;
  }

  bool ifdef(const Ast &ast, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, src->start);
    }
    return ast.macros.find(id->name) != ast.macros.end();
  }

  static ItSource include1(Ast &ast, const char *start, const UNode &src);

  template<>
  int preprocDir<"#include"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    auto itSrc = include1(ast, start, *it);
    auto &body = itSrc->second.second;
    return preproc(dest, ast, body.begin(), body.end());
  }

  static bool getFile(filesystem::path &dest, const UNode &src);

  ItSource include1(Ast &ast, const char *start, const UNode &src) {
    filesystem::path file;
    if (!getFile(file, src)) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, src->start);
    }
    if (auto itSrc = whichSource(ast, start); itSrc != ast.sources.end()) {
      file = itSrc->first / file;
    }
    try {
      file = filesystem::canonical(file);
    } catch (filesystem::filesystem_error) {
      throw AstBad(bad::CANNOT_OPEN_SRC_FILE, start);
    }
    auto itSrc = ast.sources.find(file);
    if (itSrc == ast.sources.end()) {
      itSrc = load(ast, start, std::move(file));
    }
    return itSrc;
  }

  bool getFile(filesystem::path &dest, const UNode &src) {
    if (auto a = dynamic_cast<const CharAtom *>(src.get()); a) {
      dest = filesystem::path(string(1, a->value));
      return true;
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.get()); a) {
      dest = filesystem::path(*a->value);
      return true;
    }
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest = filesystem::path(*a->name);
      return true;
    }
    if (auto a = dynamic_cast<const NumberAtom *>(src.get()); a) {
      dest = filesystem::path(a->raw);
      return true;
    }
    return false;
  }

  template<>
  int preprocDir<"#undef"_token>(UNodes &dest, Ast &ast, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    auto id = dynamic_cast<const IDAtom *>(it->get());
    if (!id) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, (**it).start);
    }
    ast.macros.erase(id->name);
    return 0;
  }
}
