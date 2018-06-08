#pragma once

#include <cstdint>

using uintmax_t = std::uintmax_t;

namespace rss {
  uintmax_t peak (void);
  uintmax_t current (void);
};
