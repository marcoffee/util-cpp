#pragma once

#include <algorithm>
#include <cstdint>

namespace util::pointer {

  template <typename T>
  T* duplicate (T const* ptr, uintmax_t size) {
    // Duplicate a pointer, given its size
    T* copy = nullptr;

    if (ptr != nullptr and size != 0) {
      copy = new T[size];
      std::copy_n(ptr, size, copy);
    }

    return copy;
  }

};
