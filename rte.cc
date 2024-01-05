#include"rte.hh"

using namespace std;

namespace zlt::mylisp::rte {
  Coroutines coroutines;
  set<filesystem::path> files;
  set<string> fnBodies;
  map<const wstring *, Value> globalDefs;
  ItCoroutine itCoroutine;
  set<string> latin1s;
  map<const wstring *, Macro> macros;
  set<ast::Pos> positions;
  set<wstring> strings;
}
