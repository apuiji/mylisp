#include<algorithm>
#include"gc_wb.hh"
#include"lists.hh"

using namespace std;

namespace zlt::mylisp {
  Value ListObj::objGetMemb(const Value &memb) const noexcept {
    int i;
    if (!dynamicast(i, memb)) {
      return Null();
    }
    if (!(i >= 0 && i < list.size())) {
      return Null();
    }
    return list[i];
  }

  int ListObj::objSetMemb(const Value &memb, const Value &value) {
    int i;
    if (!dynamicast(i, memb)) {
      return 0;
    }
    if (!(i >= 0 && i < list.size())) {
      return 0;
    }
    list[i] = value;
    gc::iwb(this, value);
    return 0;
  }

  int ListObj::graySubjs() noexcept {
    for_each(list.begin(), list.end(), gc::grayValue);
    return 0;
  }
}
