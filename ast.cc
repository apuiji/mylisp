#include<sstream>
#include"ast_load.hh"
#include"ast_optimize.hh"
#include"ast_preproc.hh"
#include"ast_trans2.hh"
#include"myccutils/xyz.hh"

using namespace std;

namespace zlt::mylisp::ast {
  using Loadeds = map<const filesystem::path *, UNode>;

  static UNode &load1(Loadeds &loadeds, filesystem::path &&file);

  int ast(UNode &dest, const filesystem::path &file) {
    Loadeds loadeds;
    auto a = 
    preproc(src, remove(src));
    trans(src, remove(src));
    optimize(src);
    trans1(src);
    trans2(src);
    return 0;
  }

  UNode &load1() {
    filesystem::path *file1;
    UNode a;
    try {
      file1 = load(a, std::move(file));
    } catch (LoadBad bad) {
      throw PreprocBad(std::move(bad.what), pos);
    } catch (ParseBad bad) {
      wstring postr;
      pos2str(postr, bad.pos);
      throw PreprocBad(std::move(bad.what) + postr, pos);
    } catch (PreprocBad bad) {
      throw PreprocBad(std::move(bad), pos);
    }
    return loadeds[file1] = std::move(a);
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
