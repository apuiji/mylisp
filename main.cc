#include<filesystem>
#include<iostream>
#include"compile.hh"
#include"eval.hh"
#include"myccutils/xyz.hh"
#include"rte.hh"

using namespace std;

static const char *indexFile;

static int parseOptions(const char **it, const char **end);

int main(int argc, char **argv, char **envp) {
  parseOptions(argv + 1, argv + argc);
  rte::init();
  if (indexFile) {
    ast::UNode a;
    ast::include(a, filesystem::path(indexFile));
    ast::ast(a);
    string s;
    compile(s, a);
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
  indexFile = *it;
}
