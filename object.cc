#include<algorithm>
#include<cmath>
#include"gc_wb.hh"
#include"myccutils/myiter.hh"

using namespace std;

namespace zlt::mylisp {
  int FunctionObj::graySubjs() noexcept {
    myiter::forEach(myiter::makeElementAtRange<1>(closures), gc::grayValue);
    return 0;
  }

  int ValueObj::graySubjs() noexcept {
    gc::grayValue(value);
    return 0;
  }
}
