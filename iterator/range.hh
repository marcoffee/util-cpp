#pragma once

#include <iterator>

namespace util::iterator {

  template <typename T = intmax_t>
  class range {

  public:
    using difference_type = intmax_t;
    using value_type = T;
    using pointer = T;
    using reference = T;
    using iterator_category = std::random_access_iterator_tag;

  private:
    T _state = 0;

  public:
    constexpr explicit range (T start = 0) : _state(start) {}

    constexpr T state (void) const { return this->_state; }
    constexpr T operator * (void) { return this->state(); }

    constexpr range& operator ++ (void) {
      this->_state += 1;
      return *this;
    }

    constexpr range& operator -- (void) {
      this->_state -= 1;
      return *this;
    }

    constexpr range operator ++ (int) {
      range rng(this->_state);
      this->_state += 1;
      return rng;
    }

    constexpr range operator -- (int) {
      range rng(this->_state);
      this->_state -= 1;
      return rng;
    }

    constexpr range& operator += (difference_type d) {
      this->_state += d;
      return *this;
    }

    constexpr range& operator -= (difference_type d) {
      this->_state -= d;
      return *this;
    }

    constexpr range operator + (difference_type d) const {
      return range(this->_state + d);
    }

    constexpr range operator - (difference_type d) const {
      return range(this->_state - d);
    }

    constexpr T operator [] (difference_type d) const {
      return this->_state + d;
    }

    constexpr difference_type operator - (range const& rng) const {
      return this->_state - rng._state;
    }

    constexpr bool operator == (range const& rng) const {
      return this->_state == rng._state;
    }

    constexpr bool operator != (range const& rng) const {
      return this->_state != rng._state;
    }

    constexpr bool operator > (range const& rng) const {
      return this->_state > rng._state;
    }

    constexpr bool operator < (range const& rng) const {
      return this->_state < rng._state;
    }

    constexpr bool operator >= (range const& rng) const {
      return this->_state >= rng._state;
    }

    constexpr bool operator <= (range const& rng) const {
      return this->_state <= rng._state;
    }
  };

  template <typename T>
  constexpr range<T> operator + (intmax_t d, range<T> const& rng) {
    return range(rng.state() + d);
  }

  template <typename T>
  std::reverse_iterator<range<T>> rrange (T start) {
    return std::make_reverse_iterator(range<T>(start));
  }

};
