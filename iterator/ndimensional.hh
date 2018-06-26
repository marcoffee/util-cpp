#pragma once

#include <array>
#include <iterator>
#include <functional>

namespace util::iterator {

  #define __NDIM_IT ndimensional<D, V, PTR, REF>

  #define __NDIM_IT_FUNC(type)\
  template <uintmax_t D, typename V, typename PTR, typename REF>\
  type __NDIM_IT

  template <uintmax_t D, typename V, typename PTR = V, typename REF = V>
  class ndimensional {

  public:
    using difference_type = intmax_t;
    using value_type = V;
    using pointer = PTR;
    using reference = REF;
    using iterator_category = std::random_access_iterator_tag;
    using index = std::array<uintmax_t, D>;
    using getter = std::function<reference(index const&)>;

  private:
    getter _get;
    index _size, _idx;

  public:
    constexpr static uintmax_t absolute_pos (index const& size, index const& idx);
    constexpr static void relative_pos (uintmax_t abso, index const& size, index& out);

    constexpr ndimensional (
      getter get, index const& size, index const& idx = { 0, 0 }
    ) : _get(get), _size(size), _idx(idx) {};

    constexpr uintmax_t const absolute_pos (void) const {
      return ndimensional::absolute_pos(this->_size, this->_idx);
    }

    uintmax_t const& idx (uintmax_t dim) const { return this->_idx[dim]; }

    constexpr bool operator == (ndimensional const& ot) const;
    constexpr bool operator != (ndimensional const& ot) const {
      return !(*this == ot);
    }

    constexpr reference operator * (void) const { return this->_get(this->_idx); }

    constexpr ndimensional& operator ++ (void);
    constexpr ndimensional& operator -- (void);

    constexpr ndimensional operator ++ (int) {
      ndimensional old = *this; ++(*this); return old;
    }

    constexpr ndimensional operator -- (int) {
      ndimensional old = *this; --(*this); return old;
    }

    constexpr ndimensional& operator += (intmax_t diff);
    constexpr ndimensional& operator -= (intmax_t diff);

    constexpr ndimensional operator + (intmax_t diff) const {
      return (ndimensional(*this) += diff);
    }

    constexpr ndimensional operator - (intmax_t diff) const {
      return (ndimensional(*this) -= diff);
    }

    constexpr friend ndimensional operator + (
      intmax_t diff, ndimensional const& it
    ) { return it + diff; }

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
      if (this->_idx[i] != ot._idx[i] || this->_size[i] != ot._size[i]) {
        return false;
      }
    }

    return true;
  }

  __NDIM_IT_FUNC(constexpr __NDIM_IT&)::operator ++ (void) {
    for (uintmax_t dim = D; dim > 0; --dim) {
      uintmax_t d = dim - 1;

      if (++this->_idx[d] != this->_size[d] || d == 0) {
        break;
      }

      this->_idx[d] = 0;
    }

    return *this;
  }

  __NDIM_IT_FUNC(constexpr __NDIM_IT&)::operator -- (void) {
    for (uintmax_t dim = D; dim > 0; --dim) {
      uintmax_t d = dim - 1;

      if (this->_idx[d] != 0) {
        this->_idx[d]--;
        break;
      }

      this->_idx[d] = this->_size[d] - 1;
    }

    return *this;
  }

  __NDIM_IT_FUNC(constexpr __NDIM_IT&)::operator -= (intmax_t diff) {
    if (diff < 0) {
      return (*this += -diff);

    } else if (diff > 0) {
      uintmax_t const udiff = diff;
      uintmax_t pos = this->absolute_pos();
      pos = (udiff > pos) ? 0 : pos - udiff;
      __NDIM_IT::relative_pos(pos, this->_size, this->_idx);
    }
    return *this;
  }

  __NDIM_IT_FUNC(constexpr __NDIM_IT&)::operator += (intmax_t diff) {
    if (diff < 0) {
      return (*this -= -diff);

    } else if (diff > 0) {
      uintmax_t const pos = this->absolute_pos() + diff;
      __NDIM_IT::relative_pos(pos, this->_size, this->_idx);
    }

    return *this;
  }

  __NDIM_IT_FUNC(constexpr bool)::operator > (__NDIM_IT const& ot) const {
    for (uintmax_t i = 0; i < D; ++i) {
      if (this->_idx[i] > ot._idx[i] || this->_size[i] > ot._size[i]) {
        return true;
      }
    }

    return false;
  }

  __NDIM_IT_FUNC(constexpr bool)::operator < (__NDIM_IT const& ot) const {
    for (uintmax_t i = 0; i < D; ++i) {
      if (this->_idx[i] < ot._idx[i] || this->_size[i] < ot._size[i]) {
        return true;
      }
    }

    return false;
  }

};
