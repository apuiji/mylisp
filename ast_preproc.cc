#include<algorithm>
#include<cstring>
#include<sstream>
#include"ast_load.hh"
#include"ast_preproc.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = UNodes::const_iterator;
  using Posk = vector<const char *>;

  static int preprocList(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end);
  static int clone(UNode &dest, const UNode &src);
  static int clones(UNodes &dest, It it, It end);

  int preproc(UNodes &dest, Ast &ast, Posk &posk, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto ls = dynamic_cast<const List*>(it->get()); ls && ls->items.size()) {
      preprocList(dest, ast, posk, ls->start, ls->items.begin(), ls->items.end());
    } else {
      UNode a;
      clone(a, *it);
      dest.push_back(std::move(a));
    }
    return preproc(dest, ast, posk, it + 1, end);
  }

  using PreprocDir = int(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end);

  static PreprocDir *isPreprocDir(const UNode &src) noexcept;
  static const Macro *isMacro(const Ast &ast, const UNode &src) noexcept;
  static int macroExpand(UNodes &dest, const Macro *macro, It it, It end);

  struct PopPoskGuard {
    Posk &posk;
    PopPoskGuard(Posk &posk) noexcept: posk(posk) {}
    ~PopPoskGuard() {
      posk.pop_back();
    }
  };

  int preprocList(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    if (auto pd = isPreprocDir(*it); pd) {
      return pd(dest, ast, posk, start, it + 1, end);
    }
    if (auto m = isMacro(ast, *it); m) {
      UNodes a;
      macroExpand(a, m, it + 1, end);
      posk.push_back(start);
      PopPoskGuard g(posk);
      return preproc(dest, ast, posk, a.begin(), a.end());
    }
    UNodes items;
    preproc(items, ast, posk, it, end);
    UNode a(new List(start, std::move(items)));
    dest.push_back(std::move(a));
    return 0;
  }

  template<uint64_t T>
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

  static int addStringAtom(UNodes &dest, const char *start, string &&s) {
    auto &value = *rte::strings.insert(std::move(s)).first;
    UNode a(new StringAtom(start, &value));
    dest.push_back(std::move(a));
    return 0;
  }

  template<>
  int preprocDir<"#"_token>(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      return addStringAtom(dest, start, "");
    }
    if (auto a = dynamic_cast<const RawAtom *>(it->get()); a) {
      return addStringAtom(dest, start, a->raw);
    }
    throw AstBad(bad::UNEXPECTED_TOKEN, (**it).start);
  }

  static int rawCat(stringstream &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto a = dynamic_cast<const RawAtom *>(it->get()); a) {
      dest << a->raw;
      return rawCat(dest, it + 1, end);
    }
    throw AstBad(bad::UNEXPECTED_TOKEN, (**it).start);
  }

  template<>
  int preprocDir<"##"_token>(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    stringstream ss;
    rawCat(ss, it, end);
    auto &s = *rte::strings.insert(ss.str()).first;
    remove(ss);
    UNode a;
    if (double d; isNumber(d, s)) {
      a.reset(new NumberAtom(start, s, d));
    } else {
      a.reset(new StringAtom(start, &s));
    }
    dest.push_back(std::move(a));
    return 0;
  }

  static int makeMacroParams(Macro::Params &dest, It it, It end);

  template<>
  int preprocDir<"#def"_token>(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    auto id = dynamic_cast<const IDAtom *>(it->get());
    if (!id) {
      throw AstBad(bad::UNEXPECTED_TOKEN, (**it).start);
    }
    if (ast.macros.find(id->name) != ast.macros.end()) {
      throw AstBad(bad::MACRO_ALREADY_DEFINED, start);
    }
    It it1 = it + 1;
    if (it1 == end) {
      ast.macros[id->name] = Macro();
      return 0;
    }
    auto ls = dynamic_cast<const List *>(it1->get());
    if (!ls) {
      throw AstBad(bad::UNEXPECTED_TOKEN, (**it1).start);
    }
    Macro::Params params;
    makeMacroParams(params, ls->items.begin(), ls->items.end());
    params.shrink_to_fit();
    UNodes body;
    clones(body, it1 + 1, end);
    ast.macros[id->name] = Macro(std::move(params), std::move(body));
    return 0;
  }

  int makeMacroParams(Macro::Params &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(it->get()); id) {
      dest.push_back(id->name);
      if (!strncmp(id->name->data(), "...", 3) && (it + 1 != end)) {
        throw AstBad(bad::REST_MACRO_PARAM_MUST_BE_LAST, id->start);
      }
      return makeMacroParams(dest, it + 1, end);
    }
    if (auto ls = dynamic_cast<const List *>(it->get()); ls && ls->items.empty()) {
      dest.push_back(nullptr);
      return makeMacroParams(dest, it + 1, end);
    }
    throw PreprocBad(bad::ILLEGAL_MACRO_PARAM, (**it).start);
  }

  static bool ifdef(const Ast &ast, const char *start, It it, It end);

  template<>
  int preprocDir<"#ifdef"_token>(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    return ifdef(ast, start, it, end) ? preproc(dest, ast, posk, it + 1, end) : 0;
  }

  template<>
  int preprocDir<"#ifndef"_token>(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    return ifdef(ast, start, it, end) ? 0 : preproc(dest, ast, posk, it + 1, end);
  }

  bool ifdef(const Ast &ast, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      return false;
    }
    auto id = dynamic_cast<const IDAtom *>(it->get());
    if (!id) {
      throw AstBad(bad::UNEXPECTED_TOKEN, (**it).start);
    }
    return ast.macros.find(id->name) != ast.macros.end();
  }

  static ItLoader include(Ast &ast, const char *start, const UNode &src);

  template<>
  int preprocDir<"#include"_token>(UNodes &dest, Ast &ast, Posk &posk, const char *start, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    auto &a = include(ast, pos, *it).second;
    posk.push_back(start);
    PopPoskGuard g(posk);
    return preproc(dest, ast, posk, a.begin(), a.end());
  }

  static bool getFile(filesystem::path &dest, const UNode &src);

  ItLoader include(Ast &ast, const char *start, const UNode &src) {
    filesystem::path file;
    if (!getFile(file, src)) {
      throw AstBad(bad::UNEXPECTED_TOKEN, src->start);
    }
    if (auto file1 = whichFile(ast, start); file1) {
      file = *file1 / file;
    }
    try {
      file = filesystem::canonical(file);
    } catch (filesystem::filesystem_error) {
      throw AstBad(bad::SRC_FILE_OPEN_FAILED, start);
    }
    auto it = find_if(ast.loadeds.begin(), ast.loadeds.end(), [&file] (auto &p) { return *p.first == file; });
    if (it != ast.loadeds.end()) {
      return it;
    }
    try {
      it = load(ast, std::move(file));
    } catch (LoadBad bad) {
      throw PreprocBad(std::move(bad.what), pos);
    } catch (ParseBad bad) {
      stringstream ss;
      ss << bad.what << bad.pos;
      throw PreprocBad(ss.str(), pos);
    } catch (PreprocBad bad) {
      throw PreprocBad(std::move(bad), pos);
    }
    return itLoaded->second;
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
    return false;
  }

  struct MacroExpand {
    map<const string *, const UNode *> map;
    UNode &operator ()(UNodes &dest, const UNode &src);
  };

  static int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, const UNode &src);

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

  static int makeMacroExpand1(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam);

  int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, const UNode &src) {
    if (itParam == endParam) [[unlikely]] {
      return 0;
    }
    if (!src) [[unlikely]] {
      return makeMacroExpand1(dest, itParam, endParam);
    }
    if (*itParam) {
      dest.map[*itParam] = &src;
    }
    return makeMacroExpand(dest, itParam + 1, endParam, src->next);
  }

  int makeMacroExpand1(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam) {
    if (itParam == endParam) [[unlikely]] {
      return 0;
    }
    if (*itParam) {
      dest.map[*itParam] = nullptr;
    }
    return makeMacroExpand1(dest, itParam + 1, endParam);
  }

  UNode &MacroExpand::operator ()(UNode &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
    if (auto id = dynamic_cast<const IDAtom *>(src.get()); id) {
      auto it = map.find(id->name);
      if (it == map.end()) {
        dest.reset(new IDAtom(id->pos, id->name));
        return operator ()(dest->next, src->next);
      }
      if (!it->second) {
        return operator ()(dest, src->next);
      }
      if (!strncmp(it->first->data(), "...", 3)) {
        auto &next = clones(dest, *it->second);
        return operator ()(next, src->next);
      }
      clone(dest, *it->second);
      return operator ()(dest->next, src->next);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls) {
      UNode first;
      operator ()(first, ls->first);
      dest.reset(new List(ls->pos, std::move(first)));
      return operator ()(dest->next, src->next);
    }
    clone(dest, src);
    return operator ()(dest->next, src->next);
  }

  template<>
  UNode &preprocDir<"#undef"_token>(UNode &dest, Ast &ast, const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad("required macro name", pos);
    }
    ast.macros.erase(id->name);
    return dest;
  }
}
