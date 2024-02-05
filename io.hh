#include<istream>
#include<ostream>
#include"object.hh"

namespace zlt::mylisp {
  struct InputObj final: Object {
    std::istream &value;
    InputObj(std::istream &value) noexcept: value(value) {}
  };

  struct OutputObj final: Object {
    std::ostream &value;
    OutputObj(std::ostream &value) noexcept: value(value) {}
  };

  int output(std::ostream &dest, const Value *it, const Value *end);

  // native functions begin
  Value natfn_getc(const Value *it, const Value *end);
  Value natfn_output(const Value *it, const Value *end);
  // native functions end
}
