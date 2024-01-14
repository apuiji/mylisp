#include<fstream>
#include<iterator>
#include<sstream>
#include"ast_load.hh"
#include"ast_parse.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int readAll(wstring &dest, const filesystem::path &file);

  Ast::ItLoaded load(Ast &ast, filesystem::path &&file) {
    wstring src;
    readAll(src, file);
    auto file1 = &*ast.files.insert(std::move(file)).first;
    auto &src1 = ast.sources[file1] = std::move(src);
    UNode a;
    parse(a, ast, file1, src1.data(), src1.data() + src1.size());
    return ast.loadeds.insert(make_pair(file1, std::move(a))).first;
  }

  int readAll(wstring &dest, const filesystem::path &file) {
    wifstream ifs;
    try {
      ifs = wifstream(file);
    } catch (...) {
      throw LoadBad(L"cannot open file: " + file.wstring());
    }
    wstringstream ss;
    copy(istreambuf_iterator<wchar_t>(ifs), istreambuf_iterator<wchar_t>(), ostreambuf_iterator<wchar_t>(ss));
    ifs.close();
    dest = ss.str();
    return 0;
  }
}
