#include<sstream>
#include"ast_load.hh"
#include"ast_optimize.hh"
#include"ast_preproc.hh"
#include"ast_trans2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  int Ast::operator ()(UNode &dest, const filesystem::path &file) {
    ItSource it = load(*this, nullptr, filesystem::canonical(file));
    {
      UNode a;
      preproc(a, *this, it->second.second);
      trans(dest, a);
    }
    optimize(dest);
    trans1(dest);
    trans2(dest);
    return 0;
  }

  static ItSource whichSource1(const char *start, ItSource it, ItSource end) noexcept;

  ItSource whichSource(const Ast &ast, const char *start) noexcept {
    if (!start) [[unlikely]] {
      return ast.sources.end();
    }
    return whichSource1(start, ast.sources.begin(), ast.sources.end());
  }

  ItSource whichSource1(const char *start, ItSource it, ItSource end) noexcept {
    if (it == end) [[unlikely]] {
      return end;
    }
    auto &src = it->second.first;
    if (start >= src.data() && start < src.data() + src.size()) {
      return it;
    }
    return whichSource1(start, ++it, end);
  }
}
