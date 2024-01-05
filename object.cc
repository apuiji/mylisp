#include<algorithm>
#include"gc.hh"
#include"object.hh"

using namespace std;

namespace zlt::mylisp {
  int FunctionObj::graySubjs() noexcept {
    for_each(closures.begin(), closures.end(), gc::grayIt);
    return 0;
  }

  int ListObj::graySubjs() noexcept {
    for_each(list.begin(), list.end(), gc::grayIt);
    return 0;
  }

  int SetObj::graySubjs() noexcept {
    for_each(set.begin(), set.end(), gc::grayIt);
    return 0;
  }

  bool SetObj::Comparator::operator ()(const Value &a, const Value &b) const noexcept {
    switch (a.index()) {
      case Value::NULL_INDEX: {
        return false;
      }
      case Value::NUM_INDEX: {
        if (b.)
      }
    }
  }
}
