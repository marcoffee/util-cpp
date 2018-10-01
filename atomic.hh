#pragma once

#include <atomic>

namespace util::atomic {

  // Thanks to https://stackoverflow.com/a/16190791/6441345

  // Atomic max
  template <typename T>
  void max (std::atomic<T>& max, T const& now) noexcept {
    T old = max;
    while (old < now and !max.compare_exchange_weak(old, now));
  }

  // Atomic min
  template <typename T>
  void min (std::atomic<T>& min, T const& now) noexcept {
    T old = min;
    while (old > now and !min.compare_exchange_weak(old, now));
  }

}
