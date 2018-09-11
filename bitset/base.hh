#pragma once

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <gmp.h>
#include <gmpxx.h>
#include <random>
#include "../util_constexpr.hh"
#include "../ts_ptr.hh"
#include "macros.hh"

class bitset {
public:
  using type = uintmax_t;

private:
  uintmax_t _size = 0, _buckets = 0;
  ts_ptr<uintmax_t> _popcount;
  ts_ptr<bitset::type[]> _guard;
  bitset::type _last = bitset::ones, _mask = 0;
  bitset::type* _data = nullptr;

  void fill_data (bool zeros);
  void alloc_data (bool zeros);
  void unset_data (void);

  void copy_meta (bitset const& other);
  void set_data (ts_ptr<uintmax_t> const& popcount, ts_ptr<bitset::type[]> const& guard);
  void release_data (void);

  bitset& copy_from (bitset const& other);
  bitset& move_from (bitset& other);

  void fix_popcount (void);

public:

  enum compare {
    equal, different, bigger, smaller, unknown
  };

  constexpr static uintmax_t const bits = std::numeric_limits<bitset::type>::digits;
  constexpr static uintmax_t const bytes = bitset::bits >> 3;
  constexpr static uintmax_t const bits_mask = bitset::bits - 1;
  constexpr static uintmax_t const bits_shift = static_log2_v<bitset::bits>;
  constexpr static bitset::type const zero = 0;
  constexpr static bitset::type const one = 1;
  constexpr static bitset::type const ones = ~bitset::zero;
  constexpr static bitset const none();

  constexpr static uintmax_t get_ind (uintmax_t pos) {
    return pos >> bitset::bits_shift;
  }

  constexpr static uintmax_t get_bit (uintmax_t pos) {
    return pos & bitset::bits_mask;
  }

  constexpr static void bitset_sizes (uintmax_t size, uintmax_t& buckets, uintmax_t& last) {
    last = get_bit(size);
    buckets = get_ind(size) + (last > 0);
  }

  template <typename RND>
  static bitset random (uintmax_t size, RND& rnd);

  static bitset random (uintmax_t size) {
    std::default_random_engine rnd((std::random_device())());
    return bitset::random(size, rnd);
  }

  inline static bitset& NOT (bitset const& a, bitset& out) { return (out = ~a); }
  inline static bitset& WIRE (bitset const& a, bitset& out) { return (out = a); }

  static bitset& AND  (bitset const& a, bitset const& b, bitset& out);
  static bitset& OR   (bitset const& a, bitset const& b, bitset& out);
  static bitset& XOR  (bitset const& a, bitset const& b, bitset& out);

  static bitset& NAND (bitset const& a, bitset const& b, bitset& out);
  static bitset& NOR  (bitset const& a, bitset const& b, bitset& out);
  static bitset& XNOR (bitset const& a, bitset const& b, bitset& out);

  static bitset& MAJ  (bitset const& a, bitset const& b, bitset const& c, bitset& out);
  static bitset& MIN  (bitset const& a, bitset const& b, bitset const& c, bitset& out);

  static bitset& ITE  (bitset const& a, bitset const& b, bitset const& c, bitset& out);

  bitset (void) {}

  bitset (uintmax_t size, bool zeros = true) : _size(size) {
    this->fill_data(zeros);
  }

  bitset (bitset const& other) { this->copy_from(other); }
  bitset& operator = (bitset const& other) { return this->copy_from(other); }

  bitset (bitset&& other) { this->move_from(other); }
  bitset& operator = (bitset&& other) { return this->move_from(other); }

  ~bitset (void) { this->free(); }

  bitset copy (void) const;
  void free (void);

  bitset::type const& bucket (uintmax_t pos) const { return this->_data[pos]; }

  bool get (uintmax_t pos) const {
    return this->get(bitset::get_ind(pos), bitset::get_bit(pos));
  }

  bool get (uintmax_t ind, uintmax_t bit) const {
    return ((this->_data[ind] ^ this->mask()) >> bit) & 1;
  }

  template <bool use_mask = true>
  void set (uintmax_t pos) {
    if constexpr (use_mask) {
      if (this->mask()) {
        this->reset<false>(pos);
        return;
      }
    }

    uintmax_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
    bitset::type const sel = bitset::one << bit;

    *this->_popcount += (this->_data[ind] & sel) == 0;
    this->_data[ind] |= sel;
  }

  template <bool use_mask = true>
  void reset (uintmax_t pos) {
    if constexpr (use_mask) {
      if (this->mask()) {
        this->set<false>(pos);
        return;
      }
    }

    uintmax_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
    bitset::type const sel = bitset::one << bit;

    *this->_popcount -= (this->_data[ind] & sel) != 0;
    this->_data[ind] &= ~sel;
  }

  void flip (uintmax_t pos) {
    uintmax_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
    bitset::type const sel = bitset::one << bit;

    *this->_popcount += (this->_data[ind] & sel) ? -1 : 1;
    this->_data[ind] ^= sel;
  }

  void fill (bitset::type value) {
    uintmax_t const pop = ::popcount(value);
    std::fill_n(this->_data, this->buckets(), value ^ this->mask());
    *this->_popcount = this->buckets() * pop;
    this->fix_last<true>();
  }

  void fill (uintmax_t bucket, bitset::type value) {
    uintmax_t const old_pop = ::popcount(this->_data[bucket]);
    this->_data[bucket] = value ^ this->mask();
    *this->_popcount -= old_pop - ::popcount(value);

    if (bucket == (this->buckets() - 1)) {
      this->fix_last<true>();
    }
  }

  void fill (uintmax_t start, uintmax_t stop, bitset::type value, bool fix = true) {
    std::fill(this->_data + start, this->_data + stop, value);
    this->_mask = 0;

    if (stop == (this->buckets() - 1)) {
      this->fix_last<false>();
    }

    if (fix) {
      this->fix_popcount();
    }
  }

  void fill_n (uintmax_t start, uintmax_t count, bitset::type value, bool fix = true) {
    this->fill(start, start + count, value, fix);
  }

  void set (void) {
    std::fill(this->_data, this->_data + this->buckets(), bitset::ones);
    this->_mask = 0;
    this->fix_last<false>();
    *this->_popcount = this->size();
  }

  void reset (void) {
    std::fill(this->_data, this->_data + this->buckets(), bitset::zero);
    this->_mask = 0;
    *this->_popcount = 0;
  }

  template <bool pop>
  void fix_last (void) {
    if (!this->fit()) {
      bitset::type& back = this->_data[this->buckets() - 1];
      bitset::type const new_back = back & this->last();

      if constexpr (pop) {
        *this->_popcount -= ::popcount(back) - ::popcount(new_back);
      }

      back = new_back;
    }
  }

  bool iter_bits (uintmax_t& pos, bool& out) const;

  inline bitset& flip (void) {
    this->_mask = ~this->_mask;
    return *this;
  }

  inline bool is (bitset const& b) const {
    return this->mask() == b.mask() && this->data() == b.data();
  }

  inline compare fast_compare (bitset const& b) const {
    if (this->size() > b.size()) {
      return compare::bigger;
    }

    if (b.size() > this->size()) {
      return compare::smaller;
    }

    if (this->popcount() != b.popcount()) {
      return compare::different;
    }

    if (this->is(b) || this->popcount() == this->size() || this->popcount() == 0) {
      return compare::equal;
    }

    return compare::unknown;
  }

  bool operator [] (uintmax_t pos) const { return this->get(pos); }

  inline bitset operator |  (bitset const& other) const {
    bitset bs(this->size());
    return bitset::OR(*this, other, bs);
  }

  inline bitset operator &  (bitset const& other) const {
    bitset bs(this->size());
    return bitset::AND(*this, other, bs);
  }

  inline bitset operator ^  (bitset const& other) const {
    bitset bs(this->size());
    return bitset::XOR(*this, other, bs);
  }

  inline bitset operator ~  (void) const {
    bitset bs = *this;
    return bs.flip();
  }

  inline bitset& operator |= (bitset const& other) {
    return bitset::OR(*this, other, *this);
  }

  inline bitset& operator &= (bitset const& other) {
    return bitset::AND(*this, other, *this);
  }

  inline bitset& operator ^= (bitset const& other) {
    return bitset::XOR(*this, other, *this);
  }

  friend bool operator <  (bitset const& a, bitset const& b);
  friend bool operator == (bitset const& a, bitset const& b);

  inline uintmax_t const& size (void) const { return this->_size; }
  inline uintmax_t const& buckets (void) const { return this->_buckets; }
  inline bitset::type const& last (void) const { return this->_last; }
  inline bitset::type const& mask (void) const { return this->_mask; }
  inline bitset::type const* data (void) const { return this->_data; }
  inline bool fit (void) const { return this->last() == bitset::ones; }
  inline bool valid (void) const { return this->_data != nullptr; }

  uintmax_t const popcount (void) const {
    if (this->mask()) {
      return this->size() - *this->_popcount;
    }

    return *this->_popcount;
  }

  friend bitset* make_bits (bitset* bits, uintmax_t inputs, uintmax_t use_bits, uintmax_t const* positions);
  friend bitset* make_bits (bitset* bits, uintmax_t inputs, uintmax_t use_bits, mpz_class& status, bool const* keep);
};

std::ostream& operator << (std::ostream& out, bitset const& bs);

bitset* make_bits (bitset* bits, uintmax_t inputs, uintmax_t use_bits = 0, uintmax_t const* positions = nullptr);
bitset* make_bits (bitset* bits, uintmax_t inputs, uintmax_t use_bits, mpz_class& status, bool const* keep = nullptr);

template <typename RND>
bitset bitset::random (uintmax_t size, RND& rnd) {
  bitset bs(size, false);
  std::uniform_int_distribution<bitset::type> dist;

  for (uintmax_t i = 0; i < bs.buckets(); ++i) {
    bs._data[i] = dist(rnd);
  }

  bs.fix_last<false>();
  bs.fix_popcount();
  return bs;
}
