#include<sstream>
#include"ast_load.hh"
#include"ast_optimize.hh"
#include"ast_preproc.hh"
#include"ast_trans2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  UNode shift(UNode &src) noexcept {
    auto a = std::move(src);
    src = std::move(a->next);
    return std::move(a);
  }

  ostream &operator <<(ostream &dest, const Pos &pos) {
    return dest << "at " << pos.first->string() << ':' << pos.second;
  }

  int Ast::operator ()(UNode &dest, const filesystem::path &file) {
    Ast::ItLoaded it;
    try {
      it = load(*this, filesystem::canonical(file));
    } catch (filesystem::filesystem_error) {
      throw AstBad("cannot open file: " + file.string());
    } catch (LoadBad bad) {
      throw AstBad(std::move(bad.what));
    } catch (ParseBad bad) {
      stringstream ss;
      ss << bad.what << bad.pos;
      throw AstBad(ss.str());
    } catch (PreprocBad bad) {
      stringstream ss;
      auto a = myiter::reverseView(bad.posk);
      auto b = myiter::transformView(a, [] (auto p) { return *p; });
      ss << bad.what << b;
      throw AstBad(ss.str());
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
