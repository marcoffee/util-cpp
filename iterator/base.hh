#pragma once

#include <numeric>
#include <iostream>
#include "../util_constexpr.hh"

#define TEMPL_ITERATOR(IT) \
  typename = std::enable_if_t<is_iterator_v<IT>>, \
  typename IT ## C = typename std::iterator_traits<IT>::iterator_category, \
  typename IT ## T = typename std::iterator_traits<IT>::value_type

namespace util::iterator {

  template <typename IT, TEMPL_ITERATOR(IT)>
  std::unordered_map<ITT, uintmax_t> invert (IT beg, IT end, uintmax_t start = 0) {
    std::unordered_map<ITT, uintmax_t> result;
    result.reserve(std::distance(beg, end));

    for (; beg != end; ++start, ++beg) {
      result.emplace(*beg, start);
    }

    return result;
  }

  template <typename IT, TEMPL_ITERATOR(IT)>
  std::vector<IT> build_vector (IT beg, IT end) {
    std::vector<IT> iterators;

    for (; beg != end; ++beg) {
      iterators.emplace_back(beg);
    }

    iterators.shrink_to_fit();
    return iterators;
  }

  template <typename IT, TEMPL_ITERATOR(IT)>
  uintmax_t get_size (IT beg, IT end, std::vector<IT>& iterators) {
    if constexpr (std::is_same_v<ITC, std::random_access_iterator_tag>) {
      return std::distance(beg, end);
    } else {
      iterators = build_vector(beg, end);
      return iterators.size();
    }
  }

  template <typename T>
  using cmpfunc = std::function<bool(T, T)>;

  template <typename T>
  using ordfunc = std::function<void(cmpfunc<T> const&, uintmax_t)>;

  template <typename T>
  using idxfunc = std::function<ordfunc<T>(T*)>;

  template<
    typename OIT, TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_same_v<OITC, std::random_access_iterator_tag>>,
    typename = std::enable_if_t<std::is_integral_v<OITT>>
  >
  constexpr ordfunc<OITT> argfunc_partition (OIT& out, uintmax_t nth) {
    return [ out, nth ] (cmpfunc<OITT> const& order, uintmax_t size) {
      std::nth_element(out, out + nth, out + size, order);
    };
  }

  template<
    typename OIT, TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_same_v<OITC, std::random_access_iterator_tag>>,
    typename = std::enable_if_t<std::is_integral_v<OITT>>
  >
  constexpr ordfunc<OITT> argfunc_sort (OIT& out) {
    return [ out ] (cmpfunc<OITT> const& order, uintmax_t size) {
      std::sort(out, out + size, order);
    };
  }

  template <
    typename IT, typename OIT, TEMPL_ITERATOR(IT), TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_same_v<OITC, std::random_access_iterator_tag>>,
    typename = std::enable_if_t<std::is_integral_v<OITT>>
  >
  void argfunction (
    IT beg, IT end, OIT out, ordfunc<OITT> const& order,
    cmpfunc<ITT> const& compare = std::less<ITT>(),
    std::vector<IT> const& _iterators = {}
  ) {
    if constexpr (std::is_same_v<ITC, std::random_access_iterator_tag>) {
      uintmax_t const size = std::distance(beg, end);
      std::iota(out, out + size, 0);

      order([ &beg, &end, &compare ] (uintmax_t a, uintmax_t b) {
        return compare(*(beg + a), *(beg + b));
      }, size);

    } else {
      std::vector<IT> iters;

      if (_iterators.empty()) {
        iters = build_vector(beg, end);
      }

      std::vector<IT> const& iterators = _iterators.empty() ? iters : _iterators;
      uintmax_t const size = iterators.size();
      std::iota(out, out + size, 0);

      order([ &iterators, &compare ] (uintmax_t a, uintmax_t b) {
        return compare(*iterators[a], *iterators[b]);
      }, size);
    }
  }

  template <
    typename IT, typename OIT, TEMPL_ITERATOR(IT), TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_same_v<OITC, std::random_access_iterator_tag>>,
    typename = std::enable_if_t<std::is_integral_v<OITT>>
  >
  void argpartition (
    IT beg, IT end, OIT out, uintmax_t nth,
    cmpfunc<ITT> const& compare = std::less<ITT>(),
    std::vector<IT> const& _iterators = {}
  ) {
    argfunction(beg, end, out, argfunc_partition(out, nth), compare, _iterators);
  }

  template <
    typename IT, typename OIT, TEMPL_ITERATOR(IT), TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_same_v<OITC, std::random_access_iterator_tag>>,
    typename = std::enable_if_t<std::is_integral_v<OITT>>
  >
  void argsort (
    IT beg, IT end, OIT out,
    cmpfunc<ITT> const& compare = std::less<ITT>(),
    std::vector<IT> const& _iterators = {}
  ) {
    argfunction(beg, end, out, argfunc_sort(out), compare, _iterators);
  }

  template <
    typename IT, typename... ARGS, TEMPL_ITERATOR(IT),
    typename = std::enable_if_t<std::is_same_v<ITC, std::random_access_iterator_tag>>
  >
  constexpr void multiswap (uintmax_t p1, uintmax_t p2, IT beg, ARGS&&... args) {
    std::swap(*(beg + p1), *(beg + p2));

    if constexpr (sizeof...(ARGS) > 0) {
      multiswap(p1, p2, std::forward<ARGS>(args)...);
    }
  }

  template <
    typename IT, typename... ARGS, TEMPL_ITERATOR(IT),
    typename = std::enable_if_t<std::is_integral_v<ITT>>
  >
  void multiorder (IT beg, IT end, uintmax_t stop_at, ARGS&&... args) {
    uintmax_t const size = std::distance(beg, end);
    uintmax_t* mapper = new uintmax_t[size];

    std::iota(mapper, mapper + size, 0);

    if (stop_at == 0) {
      stop_at = size;
    }

    for (uintmax_t i = 0; i < stop_at; ++i) {
      uintmax_t const p1 = mapper[i], p2 = mapper[*(beg + i)];
      multiswap(i, p2, std::forward<ARGS>(args)...);
      mapper[p1] = p2;
      mapper[p2] = p1;
    }

    delete[] mapper;
  }

  template <typename IT, typename... ARGS, TEMPL_ITERATOR(IT)>
  void multifunction (
    cmpfunc<ITT> const& compare, IT beg, IT end, uintmax_t stop_at,
    idxfunc<uintmax_t> const& idx, ARGS&&... args
  ) {
    std::vector<IT> iterators;
    uintmax_t const size = get_size(beg, end, iterators);
    uintmax_t* afunc = new uintmax_t[size];

    argfunction(beg, end, afunc, idx(afunc), compare, iterators);
    multiorder(afunc, afunc + size, stop_at, std::forward<ARGS>(args)...);

    delete[] afunc;
  }

  template <typename RND, typename... ARGS>
  void multishuffle (uintmax_t size, RND& rnd, ARGS&&... args) {
    uintmax_t* afunc = new uintmax_t[size];

    std::iota(afunc, afunc + size, 0);
    std::shuffle(afunc, afunc + size, rnd);
    multiorder(afunc, afunc + size, size, std::forward<ARGS>(args)...);

    delete[] afunc;
  }

  template <typename IT, typename... ARGS, TEMPL_ITERATOR(IT)>
  void multipartition (
    cmpfunc<ITT> const& compare, IT beg, IT end, uintmax_t nth, ARGS&&... args
  ) {
    multifunction(compare, beg, end, nth, [ nth ] (uintmax_t* out) {
      return argfunc_partition(out, nth);
    }, std::forward<ARGS>(args)...);
  }

  template <typename IT, typename... ARGS, TEMPL_ITERATOR(IT)>
  void multisort (cmpfunc<ITT> const& compare, IT beg, IT end, ARGS&&... args) {
    multifunction(compare, beg, end, 0, [] (uintmax_t* out) {
      return argfunc_sort(out);
    }, std::forward<ARGS>(args)...);
  }

  template <typename IT, typename... ARGS, TEMPL_ITERATOR(IT)>
  void multipartition (IT beg, IT end, uintmax_t nth, ARGS&&... args) {
    std::function<bool(ITT const&, ITT const&)> const compare = std::less<ITT>();
    multipartition(compare, beg, end, nth, std::forward<ARGS>(args)...);
  }

  template <typename IT, typename... ARGS, TEMPL_ITERATOR(IT)>
  void multisort (IT beg, IT end, ARGS&&... args) {
    std::function<bool(ITT const&, ITT const&)> const compare = std::less<ITT>();
    multisort(compare, beg, end, std::forward<ARGS>(args)...);
  }

  template <
    typename VIT, typename IIT, typename OIT,
    TEMPL_ITERATOR(VIT), TEMPL_ITERATOR(IIT), TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_integral_v<IITT>>
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
    typename IIT, typename OIT, TEMPL_ITERATOR(IIT), TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_convertible_v<IITT, OITT>>
  >
  void convert (IIT ibeg, IIT iend, OIT obeg) {
    for (; ibeg != iend; ++ibeg, ++obeg) {
      *obeg = static_cast<OITT>(*ibeg);
    }
  }

  template <
    typename IIT, typename OIT, TEMPL_ITERATOR(IIT), TEMPL_ITERATOR(OIT),
    typename = std::enable_if_t<std::is_convertible_v<IITT, OITT>>
  >
  void reverse (IIT ibeg, IIT iend, OIT obeg) {
    convert(
      std::make_reverse_iterator(iend), std::make_reverse_iterator(ibeg), obeg
    );
  }

};
