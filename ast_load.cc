#include<fstream>
#include<iterator>
#include<sstream>
#include"ast_load.hh"
#include"ast_parse.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int readAll(string &dest, const char *start, const filesystem::path &file);

  Ast::ItLoaded load(Ast &ast, const char *start, filesystem::path &&file) {
    string src;
    readAll(src, start, file);
    auto file1 = &*ast.files.insert(std::move(file)).first;
    auto &src1 = ast.sources[file1] = std::move(src);
    UNode a;
    parse(a, src1.data(), src1.data() + src1.size());
    return ast.loadeds.insert(make_pair(file1, std::move(a))).first;
  }

  int readAll(string &dest, const char *start, const filesystem::path &file) {
    ifstream ifs;
    try {
      ifs = ifstream(file);
    } catch (...) {
      throw AstBad(bad::CANNOT_OPEN_SRC_FILE, start);
    }
    stringstream ss;
    copy(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>(), ostreambuf_iterator<char>(ss));
    ifs.close();
    dest = ss.str();
    return 0;
  }
}
