#include<iostream>
#include"mylisp.hh"
#include"mylispc.hh"

using namespace std;

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
    const char **argv1 = argv;
    initOpts(argv1, argv1 + argc);
  } catch (init_opts::Bad bad) {
    cerr << bad.what << endl;
    return 0;
  }
  if (srcInputFile.size()) {
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
