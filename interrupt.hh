#pragma once

#include <signal.h>

namespace util::interrupt {

  extern volatile bool killed;

  void handler (int);
  void enable (void);

};
