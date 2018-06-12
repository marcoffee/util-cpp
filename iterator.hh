#pragma once

#include <iterator>
#include <vector>
#include <unordered_map>
#include "util_constexpr.hh"

namespace util::iterator {

  template <
    typename IT,
    typename T = typename std::iterator_traits<IT>::value_type,
    typename = std::enable_if_t<is_iterator_v<IT>>
  >
  std::unordered_map<T, uintmax_t> invert (IT beg, IT end, uintmax_t start = 0) {
    std::unordered_map<T, uintmax_t> result;
    result.reserve(std::distance(beg, end));

    for (; beg != end; ++start, ++beg) {
      result.emplace(*beg, start);
    }

    return result;
  }

  template <
    typename IT,
    typename = std::enable_if_t<is_iterator_v<IT>>
  >
  std::vector<IT> build_vector (IT beg, IT end) {
    std::vector<IT> iterators;

    for (; beg != end; ++beg) {
      iterators.emplace_back(beg);
    }

    iterators.shrink_to_fit();
    return iterators;
  }

  template <
    typename IT,
    typename = std::enable_if_t<is_iterator_v<IT>>
  >
  std::vector<uintmax_t> argsort (IT beg, IT end) {
    std::vector<uintmax_t> result;

    if constexpr (std::is_same_v<
      std::iterator_traits<IT>::iterator_category,
      std::random_access_iterator_tag
    >) {
      result.resize(std::distance(beg, end));
      std::iota(result.begin(), result.end(), 0);

      std::stable_sort(result.begin(), result.end(),
        [ &beg, &end ] (uintmax_t a, uintmax_t b) {
          return *(beg + a) < *(beg + b);
        }
      );

    } else {
      std::vector<IT> const iterators = build_vector(beg, end);

      result.resize(iterators.size());
      std::iota(result.begin(), result.end(), 0);

      std::stable_sort(result.begin(), result.end(),
        [ &iterators ] (uintmax_t a, uintmax_t b) {
          return *iterators[a] < *iterators[b];
        }
      );
    }

    return result;
  }

  template <
    typename VIT,
    typename IIT,
    typename OIT,
    typename = std::enable_if_t<is_iterator_v<VIT>>,
    typename = std::enable_if_t<is_iterator_v<IIT>>,
    typename = std::enable_if_t<is_iterator_v<OIT>>,
    typename T = typename std::iterator_traits<VIT>::value_type,
    typename I = typename std::iterator_traits<IIT>::value_type,
    typename = std::enable_if_t<std::is_integral_v<I>>
  >
  void index (VIT vbeg, VIT vend, IIT ibeg, OIT out) {
    if constexpr (std::is_same_v<
      std::iterator_traits<VIT>::iterator_category,
      std::random_access_iterator_tag
    >) {

      for (VIT vit = vbeg; vit != vend; ++vit, ++ibeg, ++out) {
        *out = *(vbeg + (*ibeg));
      }

    } else {
      std::vector<VIT> const iterators = build_vector(vbeg, vend);

      for (uintmax_t i = 0; i < iterators.size(); ++i, ++ibeg, ++out) {
        *out = *iterators[*ibeg];
      }
    }
  }

  template <
    typename IIT,
    typename OIT,
    typename = std::enable_if_t<is_iterator_v<IIT>>,
    typename = std::enable_if_t<is_iterator_v<OIT>>,
    typename I = typename std::iterator_traits<IIT>::value_type,
    typename O = typename std::iterator_traits<OIT>::value_type,
    typename = std::enable_if_t<std::is_convertible_v<I, O>>
  >
  void convert (IIT ibeg, IIT iend, OIT obeg) {
    for (; ibeg != iend; ++ibeg, ++obeg) {
      *obeg = static_cast<O>(*ibeg);
    }
  }

  template <
    typename IIT,
    typename OIT,
    typename = std::enable_if_t<is_iterator_v<IIT>>,
    typename = std::enable_if_t<is_iterator_v<OIT>>,
    typename I = typename std::iterator_traits<IIT>::value_type,
    typename O = typename std::iterator_traits<OIT>::value_type,
    typename = std::enable_if_t<std::is_convertible_v<I, O>>
  >
  void reverse (IIT ibeg, IIT iend, OIT obeg) {
    convert(
      std::make_reverse_iterator(iend), std::make_reverse_iterator(ibeg), obeg
    );
  }

};
