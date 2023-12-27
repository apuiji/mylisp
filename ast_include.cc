#include<fstream>
#include<iterator>
#include<sstream>
#include"ast_include.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int readAll(wstring &dest, const filesystem::path &file);

  UNode &include(UNode &dest, const filesystem::path &file) {
    wstring src;
    readAll(src, file);
    auto &file1 = *rte::files.insert(filesystem::canonical(file)).first;
    return parse(dest, file1, src.data(), src.data() + src.size());
  }

  int readAll(wstring &dest, const filesystem::path &file) {
    wifstream ifs(file);
    if (!ifs) {
      throw ParseBad(Pos(file, 0), "invalid include file");
    }
    wstringstream ss;
    copy(istreambuf_iterator<wchar_t>(ifs), istreambuf_iterator<wchar_t>(), ostreambuf_iterator<wchar_t>(ss));
    ifs.close();
    dest = ss.str();
    return 0;
  }
}
