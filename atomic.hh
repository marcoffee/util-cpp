#pragma once

#include <atomic>

namespace util::atomic {

  template <typename T>
  void max (std::atomic<T>& max, T const& now) noexcept {
    T old = max;
    while (old < now && !max.compare_exchange_weak(old, now));
  }

  template <typename T>
  void min (std::atomic<T>& min, T const& now) noexcept {
    T old = min;
    while (old > now && !min.compare_exchange_weak(old, now));
  }

}
