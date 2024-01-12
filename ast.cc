#include<sstream>
#include"ast_optimize.hh"
#include"ast_preproc.hh"
#include"ast_trans2.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  int ast(UNode &src) {
    preproc(src, remove(src));
    trans(src, remove(src));
    optimize(src);
    trans1(src);
    trans2(src);
    return 0;
  }

  UNode shift(UNode &src) noexcept {
    auto a = std::move(src);
    src = std::move(a->next);
    return std::move(a);
  }

  int pos2str(wstring &dest, const Pos &pos) {
    wstringstream ss;
    ss << L"at " << pos.first->wstring();
    ss.put(':');
    ss << pos.second;
    dest = ss.str();
    return 0;
  }
}
