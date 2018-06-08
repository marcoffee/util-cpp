#pragma once

#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "util_constexpr.hh"
#include "stream.hh"
#include "string.hh"
#include "container.hh"
#include "ndimensional_iterator.hh"

#include "argparse/argparse.hh"
#include "bitset/bitset.hh"
#include "evolution/evolution.hh"

template <typename T>
std::unordered_map<T, uintmax_t> invert_vector (std::vector<T> const& vec) {
  std::unordered_map<T, uintmax_t> result;
  result.reserve(vec.size());

  for (uintmax_t i = 0; i < vec.size(); ++i) {
    result.emplace(vec[i], i);
  }

  return result;
}

template <typename T>
std::vector<T> subvector (std::vector<T> const& vec, intmax_t begin, intmax_t end) {
  if (begin < 0) {
    begin += vec.size();
  }

  if (end < 0) {
    end += vec.size();
  }

  return std::vector<T>(vec.begin() + begin, vec.begin() + end + 1);
}

template <typename T>
std::vector<uintmax_t> argsort (T const& vec, uintmax_t size, uintmax_t shift = 0) {
  std::vector<uintmax_t> result(size);
  std::iota(result.begin(), result.end(), shift);

  std::stable_sort(result.begin(), result.end(),
    [ &vec ] (uintmax_t a, uintmax_t b) {
      return vec[a] < vec[b];
    }
  );

  return result;
}

template <typename T, typename I>
std::vector<T> subindex (std::vector<T> const& vec, std::vector<I> const& idx) {
  std::vector<T> result;
  result.reserve(idx.size());

  for (I const& i : idx) {
    result.emplace_back(vec[i]);
  }

  return result;
}

template <typename T, typename U>
std::vector<T> convert_vector (std::vector<U> const& vec) {
  std::vector<T> result;
  result.reserve(vec.size());

  for (U const& u : vec) {
    result.emplace_back(u);
  }

  return result;
}

template <typename T>
std::vector<T> reverse_vector (std::vector<T> const& vec) {
  return std::vector<T>(vec.rbegin(), vec.rend());
}

template <typename T, typename... ARGS>
inline void real_clear (std::vector<T>& vec, ARGS&&... args) {
  vec.clear();
  vec.shrink_to_fit();

  if constexpr (sizeof...(ARGS) > 0) {
    real_clear(std::forward<ARGS>(args)...);
  }
}

template <typename T, uintmax_t C>
struct _vec_n {
  using type = std::vector<typename _vec_n<T, C - 1>::type>;
};

template <typename T>
struct _vec_n<T, 0> {
  using type = T;
};

template <typename T, uintmax_t C>
using vec_n = typename _vec_n<T, C>::type;

template <typename T, typename... ARGS>
vec_n<T, sizeof...(ARGS) + 1> make_vec_n (T val, uintmax_t size, ARGS&&... args) {
  if constexpr (sizeof...(ARGS) == 0) {
    return std::vector<T>(size, val);

  } else {
    vec_n<T, sizeof...(ARGS) + 1> result;
    result.reserve(size);

    for (uintmax_t i = 0; i < size; ++i) {
      result.emplace_back(std::move(make_vec_n(val, std::forward<ARGS>(args)...)));
    }

    return result;
  }
}

template <typename T, uintmax_t C>
std::array<uintmax_t, C> max_vec_n (vec_n<T, C> const& vec, T& val = _defref_v<T>) {
  std::array<uintmax_t, C> result;

  if constexpr (C == 1) {
    auto it = std::max_element(vec.begin(), vec.end());
    val = *it;
    result[0] = std::distance(vec.begin(), it);
  } else {
    val = std::numeric_limits<T>::lowest();

    for (uintmax_t i = 0; i < vec.size(); ++i) {
      T found;
      std::array<uintmax_t, C - 1> pos = max_vec_n<T, C - 1>(vec[i], found);

      if (found > val) {
        std::copy_n(pos.begin(), C - 1, result.begin() + 1);
        result[0] = i;
        val = std::move(found);
      }
    }
  }

  return result;
}
