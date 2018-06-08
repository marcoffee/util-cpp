#include "interrupt.hh"

namespace util::interrupt {

  bool volatile killed;
  struct sigaction old;

  void handler (int) {
    killed = true;
    sigaction(SIGINT, &old, nullptr);
    sigaction(SIGTERM, &old, nullptr);
  }

  void enable (void) {
    struct sigaction act;

    killed = false;

    act.sa_handler = handler;
    act.sa_flags = 0;

    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, &old);
    sigaction(SIGTERM, &act, &old);
  }

};
