#pragma once

#include "range.hh"

namespace util::iterator {

  template <typename T, typename I = intmax_t>
  class getter : public range<I> {

   public:
    using difference_type = intmax_t;
    using value_type = T;
    using pointer = T;
    using reference = T;
    using iterator_category = std::random_access_iterator_tag;

    getter (I pos) : range<I>(pos) {}

    virtual T get (I) = 0;

    T operator * (void) { return this->get(this->state()); }

    T operator [] (difference_type d) const {
      return this->get(this->state() + d);
    }

  };

}
