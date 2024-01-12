#include<algorithm>
#include<cwchar>
#include<sstream>
#include"ast_load.hh"
#include"ast_preproc.hh"
#include"ast_token.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using Loadeds = map<const filesystem::path *, UNode>;

  static int clone(UNode &dest, const UNode &src);
  static UNode &preprocList(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &first);

  UNode &preproc(UNode &dest, Loadeds &loadeds, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
    if (auto ls = dynamic_cast<const List*>(src.get()); ls) {
      if (ls->first) {
        auto &next = preprocList(dest, loadeds, ls->pos, ls->first);
        return preproc(next, loadeds, src->next);
      } else {
        dest.reset(new List(ls->pos));
        return preproc(dest->next, loadeds, src->next);
      }
    }
    clone(dest, src);
    return preproc(dest->next, loadeds, src->next);
  }

  static UNode &clones(UNode &dest, const UNode &src);

  int clone(UNode &dest, const UNode &src) {
    if (auto a = dynamic_cast<const NumberAtom *>(src.get()); a) {
      dest.reset(new NumberAtom(a->pos, a->raw, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const CharAtom *>(src.get()); a) {
      dest.reset(new CharAtom(a->pos, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const StringAtom *>(src.get()); a) {
      dest.reset(new StringAtom(a->pos, a->value));
      return 0;
    }
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      dest.reset(new IDAtom(a->pos, a->name));
      return 0;
    }
    if (auto a = dynamic_cast<const TokenAtom *>(src.get()); a) {
      dest.reset(new TokenAtom(a->pos, a->raw, a->token));
      return 0;
    }
    auto ls = static_cast<const List *>(src.get());
    UNode first;
    clones(first, ls->first);
    dest.reset(new List(ls->pos, std::move(first)));
    return 0;
  }

  UNode &clones(UNode &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return dest;
    }
    clone(dest, src);
    return clones(dest->next, src->next);
  }

  using PreprocDir = UNode &(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src);

  static PreprocDir *isPreprocDir(const UNode &src) noexcept;
  static const Macro *findMacro(const UNode &src) noexcept;

  struct MacroExpand {
    map<const wstring *, const UNode *> map;
    UNode &operator ()(UNode &dest, const UNode &src);
  };

  static int makeMacroExpand(MacroExpand &dest, Macro::ItParam itParam, Macro::ItParam endParam, const UNode &src);

  UNode &preprocList(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &first) {
    if (auto dir = isPreprocDir(first); dir) {
      return dir(dest, loadeds, pos, first->next);
    }
    if (auto m = findMacro(first); m) {
      UNode a;
      {
        MacroExpand me;
        makeMacroExpand(me, m->params.begin(), m->params.end(), first->next);
        me(a, m->body);
      }
      return preproc(dest, loadeds, a);
    }
    UNode first1;
    preproc(first1, loadeds, first);
    dest.reset(new List(pos, std::move(first1)));
    return dest->next;
  }

  static PreprocDir ppd_def, ppd_file, ppd_idcat, ppd_ifdef, ppd_ifndef, ppd_include, ppd_line, ppd_toString, ppd_undef;

  PreprocDir *isPreprocDir(const UNode &src) noexcept {
    auto a = dynamic_cast<const TokenAtom *>(src.get());
    if (!a) {
      return nullptr;
    }
    switch (a->token) {
      #define casePPD(name) \
      case token::PPD_##name: { \
        return ppd_##name; \
      }
      casePPD(def);
      casePPD(file);
      casePPD(idcat);
      casePPD(ifdef);
      casePPD(ifndef);
      casePPD(include);
      casePPD(line);
      casePPD(toString);
      casePPD(undef);
      #undef casePPD
      default: {
        return nullptr;
      }
    }
  }

  const Macro *findMacro(const UNode &src) noexcept {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      return nullptr;
    }
    auto it = rte::macros.find(id->name);
    if (it == rte::macros.end()) {
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
      if (!wcsncmp(it->first->data(), L"...", 3)) {
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

  static int makeMacroParams(Macro::Params &dest, const UNode &src);

  UNode &ppd_def(UNode &dest, const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad(L"required macro name", pos);
    }
    if (rte::macros.find(id->name) != rte::macros.end()) {
      throw PreprocBad(L"macro already defined", pos);
    }
    auto ls = dynamic_cast<const List *>(src->next.get());
    if (!ls) {
      throw PreprocBad(L"required macro parameter list", pos);
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
        throw PreprocBad(L"rest parameter must be last", id->pos);
      }
      return makeMacroParams(dest, src->next);
    }
    if (auto ls = dynamic_cast<const List *>(src.get()); ls && !ls->first) {
      dest.push_back(nullptr);
      return makeMacroParams(dest, src->next);
    }
    throw PreprocBad(L"illegal macro parameter", src->pos);
  }

  UNode &ppd_file(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    auto &file = *rte::strings.insert(pos->first->wstring()).first;
    dest.reset(new StringAtom(pos, &file));
    return dest->next;
  }

  static int idcat(wstringstream &dest, const UNode &src);

  UNode &ppd_idcat(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    wstringstream ss;
    idcat(ss, pos, src);
    auto &name = *rte::strings.insert(ss.str()).first;
    dest.reset(new IDAtom(pos, &name));
    return dest->next;
  }

  int idcat(wstringstream &dest, const UNode &src) {
    if (!src) [[unlikely]] {
      return 0;
    }
    if (auto id = dynamic_cast<const IDAtom *>(src.get()); id) {
      dest << *id->name;
      return idcat(dest, src->next);
    }
    if (auto r = dynamic_cast<const RawAtom *>(src.get()); r) {
      dest << r->raw;
      return idcat(dest, pos, src->next);
    }
    throw PreprocBad(L"illegal identifier concat token", src->pos);
  }

  static bool ifdef(const Pos *pos, const UNode &src);

  UNode &ppd_ifdef(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    return ifdef(pos, src) ? preproc(dest, loadeds, src->next) : dest;
  }

  UNode &ppd_ifndef(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    return ifdef(pos, src) ? dest : preproc(dest, loadeds, src->next);
  }

  bool ifdef(const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad(L"required macro name", pos);
    }
    return rte::macros.find(id->name) != rte::macros.end();
  }

  static UNode &include(Loadeds &loadeds, const Pos *pos, const UNode &src);

  UNode &ppd_include(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    auto &a = include(loadeds, pos, src);
    return preproc(dest, loadeds, a);
  }

  static bool getFile(filesystem::path &dest, const Pos *pos, const UNode &src);

  UNode &include(Loadeds &loadeds, const Pos *pos, const UNode &src) {
    filesystem::path file;
    if (!getFile(file, src)) {
      throw PreprocBad(L"required include path", pos);
    }
    auto it = find_if(loadeds.begin(), loadeds.end(), [&file] (auto &p) { return *p.first == file; });
    if (it != loadeds.end()) {
      return it->second;
    }
    filesystem::path *file1;
    UNode a;
    try {
      file1 = load(a, std::move(file));
    } catch (LoadBad bad) {
      throw PreprocBad(std::move(bad.what), pos);
    } catch (ParseBad bad) {
      wstring postr;
      pos2str(postr, bad.pos);
      throw PreprocBad(std::move(bad.what) + postr, pos);
    } catch (PreprocBad bad) {
      throw PreprocBad(std::move(bad), pos);
    }
    return loadeds[file1] = std::move(a);
  }

  static bool getPath(filesystem::path &dest, const UNode &src);

  bool getFile(filesystem::path &dest, const Pos *pos, const UNode &src) {
    if (getPath(dest, src)) {
      dest = filesystem::canonical(*pos->first / dest);
      return true;
    } else {
      return false;
    }
  }

  bool getPath(filesystem::path &dest, const UNode &src) {
    if (auto a = dynamic_cast<const CharAtom *>(src.get()); a) {
      dest = filesystem::path(wstring(1, a->value));
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

  UNode &ppd_line(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    static const wchar_t *raw = L"#line";
    dest.reset(new NumberAtom(pos, wstring_view(raw, 5), pos->second));
    return dest->next;
  }

  static wstring_view toString(const UNode &src);

  UNode &ppd_toString(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    auto s = toString(src);
    if (s.size() == 1) {
      dest.reset(new CharAtom(pos, s[0]));
    } else {
      auto &value = *rte::strings.insert(toString(src));
      dest.reset(new StringAtom(pos, &value));
    }
    return dest->next;
  }

  wstring_view toString(const UNode &src) {
    if (!src) [[unlikely]] {
      return {};
    }
    if (auto a = dynamic_cast<const RawAtom *>(src.get()); a) {
      return a->raw;
    }
    if (auto a = dynamic_cast<const IDAtom *>(src.get()); a) {
      return *a->name;
    }
    throw PreprocBad(L"illegal toString token");
  }

  UNode &ppd_undef(UNode &dest, Loadeds &loadeds, const Pos *pos, const UNode &src) {
    auto id = dynamic_cast<const IDAtom *>(src.get());
    if (!id) {
      throw PreprocBad(L"required macro name", pos);
    }
    rte::macros.erase(id->name);
    return dest;
  }
}
