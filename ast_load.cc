#include<fstream>
#include<iterator>
#include<sstream>
#include"ast_load.hh"
#include"ast_parse.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int readAll(string &dest, const char *start, const filesystem::path &file);

  ItSource load(Ast &ast, const char *start, filesystem::path &&file) {
    if (auto itSrc = ast.sources.find(file); itSrc != ast.sources.end()) {
      return itSrc;
    }
    pair<string, UNode> a;
    readAll(a.first, start, file);
    parse(a.second, a.first.data(), a.first.data() + a.first.size());
    return ast.sources.insert(make_pair(std::move(file), std::move(a))).first;
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
