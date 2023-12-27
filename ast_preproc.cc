#include<cwchar>
#include"ast_preproc.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int clone(UNode &dest, const UNode &src);
  static UNode &preprocList(UNode &dest, const filesystem::path &file, const Pos *pos, const UNode &first);

  UNode &preproc(UNode &dest, const filesystem::path &file, const UNode &src) {
    if (Dynamicastable<NumberAtom, CharAtom, StringAtom, Latin1Atom, IDAtom, TokenAtom> {}(*src)) {
      clone(dest, src);
      return preproc(dest->next, file, src->next);
    }
    if (auto ls = dynamic_cast<const List*>(src.get()); ls) {
      if (ls->first) {
        auto &next = preprocList(dest, file, ls->pos, ls->first);
        return preproc(next, file, src->next);
      } else {
        dest.reset(new List(ls->pos));
        return preproc(dest->next, file, src->next);
      }
    }
    return dest;
  }

  static int clones(UNode &dest, const UNode &src);

  int clone(UNode &dest, const UNode &src) {
    #define ifLiteralAtom(T) \
    if (auto a = dynamic_cast<const LiteralAtom<T> *>(src.get()); a) { \
      dest.reset(new LiteralAtom<T>(a->pos, a->value)); \
      return 0; \
    }
    ifLiteralAtom(double);
    ifLiteralAtom(wchar_t);
    ifLiteralAtom(const std::wstring &);
    ifLiteralAtom(const std::string &);
    #undef ifLiteralAtom
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest.reset(new IDAtom(a->pos, a->name));
      return 0;
    }
    if (auto a = dynamic_cast<const TokenAtom *>(src.get()); a) {
      dest.reset(new TokenAtom(a->pos, a->token));
      return 0;
    }
    auto ls = static_cast<const List *>(src.get());
    UNode first;
    clones(first, ls->first);
    dest.reset(new List(ls->pos, std::move(first)));
    return 0;
  }

  int clones(UNode &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    clone(dest, src);
    return clones(dest->next, src->next);
  }

  using PreprocDir = UNode &(UNode &dest, const filesystem::path &file, const Pos *pos, const UNode &src);

  static PreprocDir *isPreprocDir(const UNode &src) noexcept;
  static const Macro *findMacro(const UNode &src) noexcept;

  struct MacroExpand {
    map<const wstring *, const UNode *> map;
    UNode &operator ()(UNode &dest, const filesystem::path &file, const UNode &src);
  };

  static int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, const UNode &src);

  UNode &preprocList(UNode &dest, const filesystem::path &file, const Pos *pos, const UNode &first) {
    if (auto dir = isPreprocDir(first); dir) {
      return dir(dest, file, pos, first->next);
    }
    if (auto m = findMacro(first); m) {
      MacroExpand me;
      makeMacroExpand(me, m->params.begin(), m->params.end(), first->next);
      return me(dest, file, m->body);
    }
    UNode first1;
    clones(first1, first);
    dest.reset(new List(pos, std::move(first1)));
    return dest->next;
  }

  static PreprocDir preprocDir_def, preprocDir_ifndef, preprocDir_include, preprocDir_undef;

  PreprocDir *isPreprocDir(const UNode &src) noexcept {
    auto a = dynamic_cast<const TokenAtom *>(src.get());
    if (!a) {
      return nullptr;
    }
    switch (a->token) {
      case token::DIR_def: {
        return preprocDir_def;
      }
      case token::DIR_ifndef: {
        return preprocDir_ifndef;
      }
      case token::DIR_include: {
        return preprocDir_include;
      }
      case token::DIR_undef: {
        return preprocDir_undef;
      }
      default: {
        return nullptr;
      }
    }
  }

  static int makeMacroParams(Macro::Params &dest, const UNode &src);

  UNode &preprocDir_def(UNode &dest, const filesystem::path &file, const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad(pos, "required macro name");
    }
    if (rte::macros.find(id->name) != rte::macros.end()) {
      throw PreprocBad(pos, "macro already defined");
    }
    auto ls = dynamic_cast<const List *>(src->next.get());
    if (!ls) {
      throw PreprocBad(pos, "required macro parameter list");
    }
    Macro::Params params;
    makeMacroParams(params, ls->first);
    params.shrink_to_fit();
    UNode body;
    clones(body, src->next->next);
    rte::macros[id->name] = Macro(std::move(params), std::move(body));
    return dest;
  }

  int makeMacroParams(Macro::Params &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(src.get()); id) {
      dest.push_back(id->name);
      if (!wcsncmp(id->name->data(), L"...", 3) && src->next) {
        throw PreprocBad(id->pos, "rest parameter must be last");
      }
      return makeMacroParams(dest, src->next);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls && !ls->first) {
      dest.push_back(nullptr);
      return makeMacroParams(dest, src->next);
    }
    throw PreprocBad(src->pos, "illegal macro parameter");
  }

  UNode &preprocDir_ifndef(UNode &dest, const filesystem::path &file, const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad(pos, "required macro name");
    }
    if (rte::macros.find(id->name) != rte::macros.end()) {
      return dest;
    }
    return preproc(dest, file, src->next);
  }

  static bool getPath(filesystem::path &dest, const UNode &src);

  UNode &preprocDir_include(UNode &dest, const filesystem::path &file, const Pos *pos, const UNode &src) {
    filesystem::path path;
    if (!getPath(path, src)) {
      throw PreprocBad(pos, "required include path");
    }
    UNode a;
    auto file1 = file / path;
    include(a, file1);
    return preproc(dest, file1, a);
  }

  bool getPath(filesystem::path &dest, const UNode &src) {
    if (auto a = dynamic_cast<const CharAtom *>(src.get()); a) {
      dest = filesystem::path(wstring_view(&a->value, 1));
      return true;
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.get()); a) {
      dest = filesystem::path(*a->value);
      return true;
    }
    if (auto a = dynamic_cast<const Latin1Atom *>(src.get()); a) {
      dest = filesystem::path(*a->value);
      return true;
    }
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest = filesystem::path(*a->name);
      return true;
    }
    return false;
  }

  UNode &preprocDir_undef(UNode &dest, const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad(pos, "required macro name");
    }
    rte::macros.erase(id->name);
    return dest;
  }
}
