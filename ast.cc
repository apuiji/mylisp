#include<sstream>
#include"ast_nodes2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int ast(UNodes &dest, Ast &ast, UNodes::const_iterator it, UNodes::const_iterator end);

  int Ast::operator ()(UNodes &dest, const filesystem::path &file) {
    ItSource it = load(*this, nullptr, filesystem::canonical(file));
    auto &a = it->second.second;
    return ast(dest, *this, a.begin(), a.end());
  }

  int Ast::operator ()(UNodes &dest, const char *it, const char *end) {
    UNodes a;
    parse(a, it, end);
    return ast(dest, *this, a.begin(), a.end());
  }

  int ast(UNodes &dest, Ast &ast, UNodes::const_iterator it, UNodes::const_iterator end) {
    UNodes a;
    preproc(a, ast, it, end);
    trans(dest, a.begin(), a.end());
    a = std::move(dest);
    optimize(a.begin(), a.end());
    optimizeBody(dest, a.begin(), a.end());
    trans1(dest.begin(), dest.end());
    trans2(dest.begin(), dest.end());
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
