#pragma once

#include <algorithm>

namespace util::pointer {

  template <typename T>
  T* duplicate (T const* ptr, uintmax_t size) {
    T* copy = nullptr;

    if (ptr != nullptr && size != 0) {
      copy = new T[size];
      std::copy_n(ptr, size, copy);
    }

    return copy;
  }

};
