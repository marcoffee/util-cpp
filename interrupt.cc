#include <csignal>
#include "interrupt.hh"

namespace util::interrupt {

  bool volatile killed;
  static struct sigaction old;

  // New handler to detect interruptions
  void handler (int) {
    killed = true;
    sigaction(SIGINT, &old, nullptr);
    sigaction(SIGTERM, &old, nullptr);
  }

  // Enables interruption detector
  void enable (void) {
    // Creates new signal handler
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;

    // Sets flag to false
    killed = false;

    // Updates SIGINT and SIGTERM handlers
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, &old);
    sigaction(SIGTERM, &act, &old);
  }

};
