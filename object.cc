#include<algorithm>
#include<cmath>
#include"gc_wb.hh"

using namespace std;

namespace zlt::mylisp {
  int FunctionObj::graySubjs() noexcept {
    for (auto &p : closures) {
      gc::grayValue(p.second);
    }
    return 0;
  }

  int ValueObj::graySubjs() noexcept {
    gc::grayValue(value);
    return 0;
  }
}
