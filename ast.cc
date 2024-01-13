#include<sstream>
#include"ast_load.hh"
#include"ast_optimize.hh"
#include"ast_preproc.hh"
#include"ast_trans2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  int pos2str(wstring &dest, const Pos &pos) {
    wstringstream ss;
    ss << L"at " << pos.first->wstring();
    ss.put(':');
    ss << pos.second;
    dest = ss.str();
    return 0;
  }

  UNode shift(UNode &src) noexcept {
    auto a = std::move(src);
    src = std::move(a->next);
    return std::move(a);
  }

  int Ast::operator ()(UNode &dest, const filesystem::path &file) {
    Ast::ItLoaded it;
    try {
      it = load(*this, filesystem::canonical(file));
    } catch (LoadBad bad) {
      throw AstBad(std::move(bad.what));
    } catch (ParseBad bad) {
      wstring postr;
      pos2str(postr, bad.pos);
      throw AstBad(bad.what + postr);
    } catch (PreprocBad bad) {
      wstringstream ss;
      poss2str(ss, bad.posk.rbegin(), bad.posk.rend());
      throw AstBad(bad.what + ss.str());
    }
    {
      UNode a;
      preproc(a, *this, it->second);
      trans(dest, a);
    }
    sources.clear();
    loadeds.clear();
    optimize(dest);
    trans1(dest);
    trans2(dest);
    return 0;
  }
}
