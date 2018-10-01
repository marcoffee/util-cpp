#pragma once

namespace util::interrupt {

  // Tries to simulate python's KeyboardInterrupt
  extern volatile bool killed;
  void enable (void);

};
