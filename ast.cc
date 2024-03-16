#include<sstream>
#include"ast_load.hh"
#include"ast_optimize.hh"
#include"ast_preproc.hh"
#include"ast_trans2.hh"

using namespace std;

namespace zlt::mylisp::ast {
  int Ast::operator ()(UNode &dest, const filesystem::path &file) {
    ItLoaded it;
    try {
      it = load(*this, filesystem::canonical(file));
    } catch (filesystem::filesystem_error) {
      throw AstBad(bad::SRC_FILE_OPEN_FAILED);
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
