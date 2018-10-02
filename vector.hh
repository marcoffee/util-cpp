#pragma once

#include <algorithm>
#include "iterator/base.hh"

namespace util::vector {

  // Clear vector data and releases its memory
  template <typename T>
  void real_clear (T& vec) {
    vec.clear();
    vec.shrink_to_fit();
  }

  // Multi dimension vector
  template <typename T, uintmax_t C>
  struct multi_st {
    using type = std::vector<typename multi_st<T, C - 1>::type>;
  };

  template <typename T>
  struct multi_st<T, 0> { using type = T; };

  template <typename T, uintmax_t C>
  using multi = typename multi_st<T, C>::type;

  // Make a multi dimension vector
  template <typename T, typename... ARGS>
  multi<T, sizeof...(ARGS) + 1> make_multi (T val, uintmax_t size, ARGS&&... args) {
    if constexpr (sizeof...(ARGS) == 0) {
      // Last level
      return std::vector<T>(size, val);

    } else {
      // 'Recursive' step
      return multi<T, sizeof...(ARGS) + 1>{
        size, make_multi(val, std::forward<ARGS>(args)...)
      };
    }
  }

};
