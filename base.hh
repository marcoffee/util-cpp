#pragma once

#include <atomic>

namespace util {

  template<typename T>
  void atomic_max (std::atomic<T>& max, T const& now) noexcept {
      T old = max;
      while (old < now && !max.compare_exchange_weak(old, now));
  }

  template<typename T>
  void atomic_min (std::atomic<T>& min, T const& now) noexcept {
      T old = min;
      while (old > now && !min.compare_exchange_weak(old, now));
  }

};
