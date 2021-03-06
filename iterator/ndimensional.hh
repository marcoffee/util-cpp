#pragma once

#include <array>
#include <iterator>
#include <functional>

namespace util::iterator {

  #define __NDIM_IT ndimensional<T, D, V, PTR, REF>

  #define __NDIM_IT_FUNC(type)\
  template <typename T, uintmax_t D, typename V, typename PTR, typename REF> \
  type __NDIM_IT

  template <typename T, uintmax_t D, typename V, typename PTR = V, typename REF = V>
  class ndimensional {

   public:
    using difference_type = intmax_t;
    using value_type = V;
    using pointer = PTR;
    using reference = REF;
    using iterator_category = std::random_access_iterator_tag;
    using index = std::array<uintmax_t, D>;

   private:
    index size_, idx_;

   public:
    constexpr static uintmax_t absolute_pos (index const& size, index const& idx);
    constexpr static void relative_pos (uintmax_t abso, index const& size, index& out);

    constexpr ndimensional (
      index const& size, index const& idx = { 0, 0 }
    ) : size_{ size }, idx_{ idx } {};

    constexpr uintmax_t const absolute_pos (void) const {
      return ndimensional::absolute_pos(this->size_, this->idx_);
    }

    virtual reference get (index const& idx) const = 0;
    uintmax_t const& idx (uintmax_t dim) const { return this->idx_[dim]; }

    constexpr bool operator == (ndimensional const& ot) const;
    constexpr bool operator != (ndimensional const& ot) const {
      return !(*this == ot);
    }

    constexpr reference operator * (void) const { return this->get(this->idx_); }

    constexpr T& operator ++ (void);
    constexpr T& operator -- (void);

    constexpr T operator ++ (int) {
      T old = static_cast<T&>(*this); ++(*this); return old;
    }

    constexpr T operator -- (int) {
      T old = static_cast<T&>(*this); --(*this); return old;
    }

    constexpr T& operator += (intmax_t diff);
    constexpr T& operator -= (intmax_t diff);

    constexpr T operator + (intmax_t diff) const {
      return (T{ static_cast<T const&>(*this) } += diff);
    }

    constexpr T operator - (intmax_t diff) const {
      return (T{ static_cast<T const&>(*this) } -= diff);
    }

    constexpr friend T operator + (intmax_t diff, ndimensional const& it) {
      return static_cast<T const&>(it) + diff;
    }

    constexpr V operator [] (intmax_t diff) const { return *(*this + diff); }

    constexpr difference_type operator - (ndimensional const& ot) const {
      return static_cast<intmax_t>(this->absolute_pos()) - ot.absolute_pos();
    }

    constexpr bool operator > (ndimensional const& ot) const;
    constexpr bool operator < (ndimensional const& ot) const;

    constexpr bool operator >= (ndimensional const& ot) const {
      return !(*this < ot);
    }

    constexpr bool operator <= (ndimensional const& ot) const {
      return !(*this > ot);
    }
  };

  __NDIM_IT_FUNC(constexpr uintmax_t)::absolute_pos (
    index const& size, index const& idx
  ) {
    uintmax_t result = idx[D - 1];

    for (uintmax_t i = 0; i < D - 1; ++i) {
      result += idx[i] * size[i + 1];
    }

    return result;
  }

  __NDIM_IT_FUNC(constexpr void)::relative_pos (
    uintmax_t abso, index const& size, index& out
  ) {
    for (uintmax_t i = 0; i < D - 1; ++i) {
      out[i] = abso / size[i + 1];
      abso %= size[i + 1];
    }

    out[D - 1] = abso;
  }

  __NDIM_IT_FUNC(constexpr bool)::operator == (__NDIM_IT const& ot) const {
    for (uintmax_t i = 0; i < D; ++i) {
      if (this->idx_[i] != ot.idx_[i] or this->size_[i] != ot.size_[i]) {
        return false;
      }
    }

    return true;
  }

  __NDIM_IT_FUNC(constexpr T&)::operator ++ (void) {
    for (uintmax_t dim = D; dim > 0; --dim) {
      uintmax_t d = dim - 1;

      if (++this->idx_[d] != this->size_[d] or d == 0) {
        break;
      }

      this->idx_[d] = 0;
    }

    return static_cast<T&>(*this);
  }

  __NDIM_IT_FUNC(constexpr T&)::operator -- (void) {
    for (uintmax_t dim = D; dim > 0; --dim) {
      uintmax_t d = dim - 1;

      if (this->idx_[d] != 0) {
        this->idx_[d]--;
        break;
      }

      this->idx_[d] = this->size_[d] - 1;
    }

    return static_cast<T&>(*this);
  }

  __NDIM_IT_FUNC(constexpr T&)::operator -= (intmax_t diff) {
    if (diff < 0) {
      return (*this += -diff);

    } else if (diff > 0) {
      uintmax_t const udiff = diff;
      uintmax_t pos = this->absolute_pos();
      pos = (udiff > pos) ? 0 : pos - udiff;
      __NDIM_IT::relative_pos(pos, this->size_, this->idx_);
    }
    return static_cast<T&>(*this);
  }

  __NDIM_IT_FUNC(constexpr T&)::operator += (intmax_t diff) {
    if (diff < 0) {
      return (*this -= -diff);

    } else if (diff > 0) {
      uintmax_t const pos = this->absolute_pos() + diff;
      __NDIM_IT::relative_pos(pos, this->size_, this->idx_);
    }

    return static_cast<T&>(*this);
  }

  __NDIM_IT_FUNC(constexpr bool)::operator > (__NDIM_IT const& ot) const {
    for (uintmax_t i = 0; i < D; ++i) {
      if (this->idx_[i] > ot.idx_[i] or this->size_[i] > ot.size_[i]) {
        return true;
      }
    }

    return false;
  }

  __NDIM_IT_FUNC(constexpr bool)::operator < (__NDIM_IT const& ot) const {
    for (uintmax_t i = 0; i < D; ++i) {
      if (this->idx_[i] < ot.idx_[i] or this->size_[i] < ot.size_[i]) {
        return true;
      }
    }

    return false;
  }

};
