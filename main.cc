#include<iostream>
#include"mylisp.hh"
#include"mylispc.hh"

using namespace std;
using namespace zlt;
using namespace zlt::mylisp;

static void initOpts(const char **it, const char **end);

namespace init_opts {
  static std::string srcInputFile;

  struct Bad {
    string what;
    Bad(string &&what) noexcept: what(std::move(what)) {}
  };
}

static void main1();

int main(int argc, char **argv, char **env) {
  try {
    auto argv1 = (const char **) argv;
    initOpts(argv1, argv1 + argc);
  } catch (init_opts::Bad bad) {
    cerr << bad.what << endl;
    return 0;
  }
  initGlobalDefs();
  if (init_opts::srcInputFile.size()) {
  } else {
    // TODO: REPL
  }
  return 0;
}

void initOpts(const char **it, const char **end) {
  if (it == end) [[unlikely]] {
    return;
  }
  const char *arg = *it;
  if (*arg == '-') {
    // TODO
    return;
  }
  if (init_opts::srcInputFile.size()) {
    throw init_opts::Bad("source input file already defined");
  }
  init_opts::srcInputFile = arg;
  initOpts(it + 1, end);
}
