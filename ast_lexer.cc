#include"ast_lexer.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using It = const wchar_t *;

  tuple<uint64_t, It, It> Lexer::operator ()(It it, It end) {
    if (it == end) [[unlikely]] {
      return ;
    }
  }
}
