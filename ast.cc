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

  static ostream &operator <<(ostream &dest, const Pos &pos) {
    return dest << "at " << pos.first->string() << ':' << pos.second;
  }

  int pos2str(string &dest, const Pos &pos) {
    stringstream ss;
    ss << pos;
    dest = ss.str();
    return 0;
  }

  template<class It>
  static int posk2str(ostream &dest, It it, It end) {
    if (it == end) [[unlikely]] {
      return 0;
    }
    dest << *it << endl;
    return posk2str(dest, ++it, end);
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
      ss << bad.pos;
      throw AstBad(bad.what + ss.str());
    } catch (PreprocBad bad) {
      stringstream ss;
      posk2str(ss, bad.posk.rbegin(), bad.posk.rend());
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
