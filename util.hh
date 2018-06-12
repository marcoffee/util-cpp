#pragma once

#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "util_constexpr.hh"
#include "base.hh"
#include "stream.hh"
#include "string.hh"
#include "iterator.hh"
#include "allocator.hh"
#include "container.hh"
#include "interrupt.hh"
#include "ndimensional_iterator.hh"

#include "argparse/argparse.hh"
#include "bitset/bitset.hh"
#include "evolution/evolution.hh"

namespace util::vector {

  template <typename T, uintmax_t C>
  struct _multi {
    using type = std::vector<typename _multi<T, C - 1>::type>;
  };

  template <typename T>
  struct _multi<T, 0> {
    using type = T;
  };

  template <typename T, uintmax_t C>
  using multi = typename _multi<T, C>::type;

  template <typename T, typename... ARGS>
  multi<T, sizeof...(ARGS) + 1> make_multi (T val, uintmax_t size, ARGS&&... args) {
    if constexpr (sizeof...(ARGS) == 0) {
      return std::vector<T>(size, val);

    } else {
      multi<T, sizeof...(ARGS) + 1> result;
      result.reserve(size);

      for (uintmax_t i = 0; i < size; ++i) {
        result.emplace_back(std::move(make_multi(val, std::forward<ARGS>(args)...)));
      }

      return result;
    }
  }

  template <typename T, uintmax_t C, uintmax_t P, bool min>
  bool _minmax_multi (multi<T, C - P> const& vec, T& val, std::array<uintmax_t, C> &result) {
    bool changed = false;

    if constexpr (C - P == 1) {
      if constexpr (min) {
        auto it = std::max_element(vec.begin(), vec.end());

        if (*it > val) {
          val = *it;
          result[P] = std::distance(vec.begin(), it);
          changed = true;
        }

      } else {
        auto it = std::min_element(vec.begin(), vec.end());

        if (*it < val) {
          val = *it;
          result[P] = std::distance(vec.begin(), it);
          changed = true;
        }
      }

    } else {
      for (uintmax_t i = 0; i < vec.size(); ++i) {
        if (_minmax_multi<T, C, P + 1, min>(vec[i], val, result)) {
          result[P] = i;
          changed = true;
        }
      }

    }

    return changed;
  }

  template <typename T, uintmax_t C>
  std::array<uintmax_t, C> min_multi (multi<T, C> const& vec, T& val) {
    std::array<uintmax_t, C> result;
    _minmax_multi<T, C, C, true>(vec, val, result);

    return result;
  }

  template <typename T, uintmax_t C>
  std::array<uintmax_t, C> max_multi (multi<T, C> const& vec, T& val) {
    std::array<uintmax_t, C> result;
    _minmax_multi<T, C, C, false>(vec, val, result);

    return result;
  }

};
