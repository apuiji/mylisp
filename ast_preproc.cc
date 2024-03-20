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
  static UNode &preprocList(UNode &dest, Ast &ast, const char *start, const UNode &first);

  UNode &preproc(UNode &dest, Ast &ast, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
    if (auto ls = dynamic_cast<const List*>(src.get()); ls) {
      if (ls->first) {
        auto &next = preprocList(dest, ast, ls->start, ls->first);
        return preproc(next, ast, src->next);
      } else {
        dest.reset(new List(ls->start));
        return preproc(dest->next, ast, src->next);
      }
    }
    clone(dest, src);
    return preproc(dest->next, ast, src->next);
  }

  using PreprocDir = UNode &(UNode &dest, Ast &ast, const char *start, const UNode &src);

  static PreprocDir *isPreprocDir(const UNode &src) noexcept;
  static const Macro *findMacro(const Ast &ast, const UNode &src) noexcept;

  struct MacroExpand {
    map<const string *, const UNode *> map;
    UNode &operator ()(UNode &dest, const UNode &src);
  };

  static int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, const UNode &src);

  UNode &preprocList(UNode &dest, Ast &ast, const char *start, const UNode &first) {
    if (auto dir = isPreprocDir(first); dir) {
      return dir(dest, ast, start, first->next);
    }
    if (auto m = findMacro(ast, first); m) {
      UNode a;
      {
        MacroExpand me;
        makeMacroExpand(me, m->params.begin(), m->params.end(), first->next);
        me(a, m->body);
      }
      return preproc(dest, ast, a);
    }
    UNode first1;
    preproc(first1, ast, first);
    dest.reset(new List(start, std::move(first1)));
    return dest->next;
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
        dest.reset(new IDAtom(id->start, id->name));
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
      dest.reset(new List(ls->start, std::move(first)));
      return operator ()(dest->next, src->next);
    }
    clone(dest, src);
    return operator ()(dest->next, src->next);
  }

  template<>
  UNode &preprocDir<"#"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    string s;
    if (!src) {
      s = "";
    } else if (auto a = dynamic_cast<const RawAtom *>(src.get()); a) {
      s = a->raw;
    } else if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      s = *a->name;
    } else {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, src->start);
    }
    auto value = rte::addString(std::move(s));
    dest.reset(new StringAtom(src->start, value));
    return dest->next;
  }

  static int idcat(ostream &dest, const UNode &src);

  template<>
  UNode &preprocDir<"##"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    stringstream ss;
    idcat(ss, src);
    auto s = ss.str();
    if (s.empty()) {
      return dest;
    }
    auto name = rte::addString(std::move(s));
    dest.reset(new IDAtom(start, name));
    return dest->next;
  }

  int idcat(ostream &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    if (auto a = dynamic_cast<const RawAtom *>(src.get()); a) {
      dest << a->raw;
    } else if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest << *a->name;
    } else {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, src->start);
    }
    return idcat(dest, src->next);
  }

  static int makeMacroParams(Macro::Params &dest, const UNode &src);

  template<>
  UNode &preprocDir<"#def"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    if (!src) {
      return dest;
    }
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, start);
    }
    if (ast.macros.find(id->name) != ast.macros.end()) {
      throw AstBad(bad::MACRO_ALREADY_EXISTS, start);
    }
    if (!src->next) {
      ast.macros[id->name] = Macro();
      return dest;
    }
    auto ls = dynamic_cast<const List *>(src->next.get());
    if (!ls) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, start);
    }
    Macro::Params params;
    makeMacroParams(params, ls->first);
    params.shrink_to_fit();
    UNode body;
    clones(body, src->next->next);
    ast.macros[id->name] = Macro(std::move(params), std::move(body));
    return dest;
  }

  int makeMacroParams(Macro::Params &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(src.get()); id) {
      dest.push_back(id->name);
      if (!strncmp(id->name->data(), "...", 3) && src->next) {
        throw AstBad(bad::MACRO_REST_PARAM_MUST_BE_LAST, id->start);
      }
      return makeMacroParams(dest, src->next);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls && !ls->first) {
      dest.push_back(nullptr);
      return makeMacroParams(dest, src->next);
    }
    throw AstBad(bad::ILLEGAL_MACRO_PARAM, src->start);
  }

  static bool ifdef(const Ast &ast, const char *start, const UNode &src);

  template<>
  UNode &preprocDir<"#ifdef"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    return src && ifdef(ast, start, src) ? preproc(dest, ast, src->next) : dest;
  }

  template<>
  UNode &preprocDir<"#ifndef"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    return src && !ifdef(ast, start, src) ? preproc(dest, ast, src->next) : dest;
  }

  bool ifdef(const Ast &ast, const char *start, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, start);
    }
    return ast.macros.find(id->name) != ast.macros.end();
  }

  static bool getFile(filesystem::path &dest, const UNode &src);

  template<>
  UNode &preprocDir<"#include"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
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
    if (auto itSrc = ast.sources.find(file); itSrc != ast.sources.end()) {
      return itSrc->second.second;
    }
    auto itSrc = load(ast, start, std::move(file));
    return preproc(dest, ast, itSrc->second.second);
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

  template<>
  UNode &preprocDir<"#undef"_token>(UNode &dest, Ast &ast, const char *start, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw AstBad(bad::ILLEGAL_PREPROC_ARG, src->start);
    }
    ast.macros.erase(id->name);
    return dest;
  }
}
