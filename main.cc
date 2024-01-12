#include<filesystem>
#include<iostream>
#include"ast_include.hh"
#include"compile.hh"
#include"eval.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;
using namespace zlt;
using namespace zlt::mylisp;

static const char *indexFile;
static const char **processArgsBegin;
static const char **processArgsEnd;
static size_t mainCoroutineValuekSize = 1 << 21;

static int parseOptions(const char **it, const char **end);

int main(int argc, char **argv, char **envp) {
  parseOptions(const_cast<const char **>(argv + 1), const_cast<const char **>(argv + argc));
  rte::init();
  if (indexFile) {
    ast::UNode a;
    ast::include(a, filesystem::path(indexFile));
    ast::ast(a);
    string s;
    compile(s, a);
    {
      unique_ptr<ValueStack> k(new(mainCoroutineValuekSize) ValueStack);
      rte::coroutines.push_back(Coroutine(std::move(k)));
    }
    rte::itCoroutine = rte::coroutines.begin();
    return eval(s.data(), s.data() + s.size());
  } else {
    // todo: repl
    return 0;
  }
}

int parseOptions(const char **it, const char **end) {
  if (it == end) [[unlikely]] {
    return 0;
  }
  if (!indexFile) {
    indexFile = *it;
    processArgsBegin = it + 1;
    processArgsEnd = end;
    return 0;
  }
  return parseOptions(it + 1, end);
}
