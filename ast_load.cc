#include<fstream>
#include<iterator>
#include<sstream>
#include"ast_load.hh"
#include"rte.hh"

using namespace std;

namespace zlt::mylisp::ast {
  static int readAll(wstring &dest, const filesystem::path &file);

  const filesystem::path *load(UNode &dest, filesystem::path &&file) {
    wstring src;
    readAll(src, file);
    auto file1 = &*rte::files.insert(std::move(file)).first;
    parse(dest, file1, src.data(), src.data() + src.size());
    return file1;
  }

  int readAll(wstring &dest, const filesystem::path &file) {
    wifstream ifs(file);
    if (!ifs) {
      throw LoadBad(file, L"invalid include file");
    }
    wstringstream ss;
    copy(istreambuf_iterator<wchar_t>(ifs), istreambuf_iterator<wchar_t>(), ostreambuf_iterator<wchar_t>(ss));
    ifs.close();
    dest = ss.str();
    return 0;
  }
}
