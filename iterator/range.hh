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
    T state_ = 0;

   public:
    constexpr explicit range (T start = 0) : state_{ start } {}

    constexpr T state (void) const { return this->state_; }
    constexpr T operator * (void) { return this->state(); }

    constexpr range& operator ++ (void) {
      this->state_ += 1;
      return *this;
    }

    constexpr range& operator -- (void) {
      this->state_ -= 1;
      return *this;
    }

    constexpr range operator ++ (int) {
      range rng(this->state_);
      this->state_ += 1;
      return rng;
    }

    constexpr range operator -- (int) {
      range rng(this->state_);
      this->state_ -= 1;
      return rng;
    }

    constexpr range& operator += (difference_type d) {
      this->state_ += d;
      return *this;
    }

    constexpr range& operator -= (difference_type d) {
      this->state_ -= d;
      return *this;
    }

    constexpr range operator + (difference_type d) const {
      return range(this->state_ + d);
    }

    constexpr range operator - (difference_type d) const {
      return range(this->state_ - d);
    }

    constexpr T operator [] (difference_type d) const {
      return this->state_ + d;
    }

    constexpr difference_type operator - (range const& rng) const {
      return this->state_ - rng.state_;
    }

    constexpr bool operator == (range const& rng) const {
      return this->state_ == rng.state_;
    }

    constexpr bool operator != (range const& rng) const {
      return this->state_ != rng.state_;
    }

    constexpr bool operator > (range const& rng) const {
      return this->state_ > rng.state_;
    }

    constexpr bool operator < (range const& rng) const {
      return this->state_ < rng.state_;
    }

    constexpr bool operator >= (range const& rng) const {
      return this->state_ >= rng.state_;
    }

    constexpr bool operator <= (range const& rng) const {
      return this->state_ <= rng.state_;
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
