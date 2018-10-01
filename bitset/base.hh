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
  using bck_t = uintmax_t;
  using siz_t = uintmax_t;
  using ref_t = uint32_t;

private:
  struct impl {
    siz_t size_ = 0;
    siz_t buckets_ = 0;
    siz_t last_mask_ = 0;
    siz_t popcount_ = 0;
    bck_t* data_ = nullptr;
    std::atomic<ref_t> ref_ = 1;
  };

  impl* impl_ = nullptr;
  bck_t mask_ = 0;

  void copy_meta (bitset const& ot);
  void release (void);

  bitset& copy_from (bitset& ot);
  bitset& move_from (bitset& ot);

  void fix_popcount (void);

  bck_t* data (void) { return this->impl_->data_; }
  bck_t& data (siz_t pos) { return this->impl_->data_[pos]; }

  bck_t* begin (void) { return this->impl_->data_; }
  bck_t* end (void) { return this->begin() + this->buckets(); }

public:
  enum class compare { equal, different, unknown };

  constexpr static siz_t const bits = std::numeric_limits<bck_t>::digits;
  constexpr static siz_t const bytes = bitset::bits >> 3;
  constexpr static siz_t const bits_mask = bitset::bits - 1;
  constexpr static siz_t const bits_shift = static_log2_v<bitset::bits>;
  constexpr static bck_t const zero = 0;
  constexpr static bck_t const one = 1;
  constexpr static bck_t const ones = ~bitset::zero;

  constexpr static siz_t get_ind (siz_t pos) {
    return pos >> bitset::bits_shift;
  }

  constexpr static siz_t get_bit (siz_t pos) {
    return pos & bitset::bits_mask;
  }

  constexpr static void bitset_sizes (siz_t size, siz_t& buckets, siz_t& last) {
    siz_t const bit = get_bit(size);
    buckets = get_ind(size) + (bit > 0);
    last = bit;
  }

  template <typename RND>
  static bitset random (siz_t size, RND& rnd);

  static bitset random (siz_t size) {
    std::default_random_engine rnd((std::random_device())());
    return bitset::random(size, rnd);
  }

  inline static bitset& NOT (bitset& a, bitset& out) { return (out = ~a); }
  inline static bitset& WIRE (bitset& a, bitset& out) { return (out = a); }

  static bitset& AND  (bitset a, bitset b, bitset& out);
  static bitset& OR   (bitset a, bitset b, bitset& out);
  static bitset& XOR  (bitset a, bitset b, bitset& out);
  static bitset& MAJ  (bitset a, bitset b, bitset c, bitset& out);
  static bitset& ITE  (bitset a, bitset b, bitset c, bitset& out);

  bitset (void) {}

  bitset (siz_t size, bool zeros = true)
  : impl_{ new impl } {
    bck_t last;
    this->impl_->size_ = size;
    bitset_sizes(size, this->impl_->buckets_, last);

    if (last) {
      this->impl_->last_mask_ = (bck_t{ 1 } << last) - 1;
    } else {
      this->impl_->last_mask_ = bitset::ones;
    }

    if (zeros) {
      this->impl_->data_ = new bck_t[size]();
      this->impl_->popcount_ = 0;
    } else {
      this->impl_->data_ = new bck_t[size];
      this->fix_popcount();
    }

    this->impl_->ref_ = 1;
  }

  bitset (bitset& ot) { this->copy_from(ot); }
  bitset& operator = (bitset& ot) { return this->copy_from(ot); }

  bitset (bitset&& ot) { this->move_from(ot); }
  bitset& operator = (bitset&& ot) { return this->move_from(ot); }

  ~bitset (void) { this->free(); }
  void free (void);

  bitset copy (void) const;

  bck_t const& bucket (siz_t pos) const { return this->impl_->data_[pos]; }

  bool get (siz_t pos) const {
    return this->get(bitset::get_ind(pos), bitset::get_bit(pos));
  }

  bool get (siz_t ind, siz_t bit) const {
    return ((this->data(ind) ^ this->mask()) >> bit) & 1;
  }

  template <bool use_mask = true>
  void set (siz_t pos) {
    if constexpr (use_mask) {
      if (this->mask()) {
        this->reset<false>(pos);
        return;
      }
    }

    siz_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
    bck_t const sel = bitset::one << bit;

    this->impl_->popcount_ += (this->data(ind) & sel) == 0;
    this->data(ind) |= sel;
  }

  template <bool use_mask = true>
  void reset (siz_t pos) {
    if constexpr (use_mask) {
      if (this->mask()) {
        this->set<false>(pos);
        return;
      }
    }

    siz_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
    bck_t const sel = bitset::one << bit;

    this->impl_->popcount_ -= (this->data(ind) & sel) != 0;
    this->data(ind) &= ~sel;
  }

  void flip (siz_t pos) {
    siz_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
    bck_t const sel = bitset::one << bit;

    this->impl_->popcount_ += (this->data(ind) & sel) ? -1 : 1;
    this->data(ind) ^= sel;
  }

  void fill (bck_t value) {
    siz_t const pop = ::popcount(value);
    std::fill(this->begin(), this->end(), value ^ this->mask());
    this->impl_->popcount_ = this->buckets() * pop;
    this->fix_last<true>();
  }

  void fill (siz_t bucket, bck_t value) {
    siz_t const old_pop = ::popcount(this->data(bucket));
    this->data(bucket) = value ^ this->mask();
    this->impl_->popcount_ -= old_pop - ::popcount(value);

    if (bucket == (this->buckets() - 1)) {
      this->fix_last<true>();
    }
  }

  void fill (siz_t start, siz_t stop, bck_t value, bool fix = true) {
    std::fill(this->data() + start, this->data() + stop, value ^ this->mask());

    if (stop >= (this->buckets() - 1)) {
      this->fix_last<false>();
    }

    if (fix) {
      this->fix_popcount();
    }
  }

  void fill_n (siz_t start, siz_t count, bck_t value, bool fix = true) {
    this->fill(start, start + count, value, fix);
  }

  void set (void) {
    std::fill(this->begin(), this->end(), ~this->mask());
    this->fix_last<false>();
    this->impl_->popcount_ = this->mask() ? 0 : this->size();
  }

  void reset (void) {
    std::fill(this->begin(), this->end(), this->mask());
    this->fix_last<false>();
    this->impl_->popcount_ = this->mask() ? this->size() : 0;
  }

  template <bool pop>
  void fix_last (void) {
    if (!this->fit()) {
      bck_t& back = this->data(this->buckets() - 1);
      bck_t const new_back = back & this->last_mask();

      if constexpr (pop) {
        this->impl_->popcount_ -= ::popcount(back) - ::popcount(new_back);
      }

      back = new_back;
    }
  }

  inline bool is (bitset const& b) const {
    return this->mask() == b.mask() and this->impl_ == b.impl_;
  }

  inline compare fast_compare (bitset const& b) const {
    siz_t const a_p = this->popcount();
    siz_t const b_p = b.popcount();

    if (this->size() > b.size() or b.size() > this->size() or a_p != b_p) {
      return compare::different;
    }

    if (this->is(b) or (a_p == b_p and (a_p == this->size() or a_p == 0))) {
      return compare::equal;
    }

    return compare::unknown;
  }

  bool operator [] (siz_t pos) const { return this->get(pos); }

  inline bitset operator |  (bitset ot) {
    bitset bs(this->size(), false);
    return bitset::OR(*this, ot, bs);
  }

  inline bitset operator &  (bitset ot) {
    bitset bs(this->size(), false);
    return bitset::AND(*this, ot, bs);
  }

  inline bitset operator ^  (bitset ot) {
    bitset bs(this->size(), false);
    return bitset::XOR(*this, ot, bs);
  }

  inline bitset& operator |= (bitset ot) {
    return bitset::OR(*this, ot, *this);
  }

  inline bitset& operator &= (bitset ot) {
    return bitset::AND(*this, ot, *this);
  }

  inline bitset& operator ^= (bitset ot) {
    return bitset::XOR(*this, ot, *this);
  }

  inline bitset operator ~  (void) {
    bitset bs = *this;
    return bs.flip();
  }

  inline bitset& flip (void) {
    this->mask_ = ~this->mask_;
    return *this;
  }

  bool operator == (bitset const& ot) const;
  explicit operator std::string (void) const;

  siz_t const size (void) const { return this->impl_->size_; }
  siz_t const buckets (void) const { return this->impl_->buckets_; }
  bck_t const& last_mask (void) const { return this->impl_->last_mask_; }
  bck_t const* data (void) const { return this->impl_->data_; }
  bck_t const& data (siz_t pos) const { return this->impl_->data_[pos]; }
  bck_t const& mask (void) const { return this->mask_; }
  bool fit (void) const { return this->last_mask() == bitset::ones; }
  bool valid (void) const { return this->data() != nullptr; }

  siz_t last_bits (void) const {
    return this->fit() ? bitset::bits : (this->size() % bitset::bits);
  }

  siz_t const popcount (void) const {
    if (this->mask()) {
      return this->size() - this->impl_->popcount_;
    }

    return this->impl_->popcount_;
  }

  static bitset* build_combinations (
    bitset* bits, siz_t inputs, siz_t use_bits = 0,
    siz_t const* positions = nullptr
  );

  static bitset* build_combinations (
    bitset* bits, siz_t inputs, siz_t use_bits, mpz_class& status,
    bool const* keep = nullptr
  );
};

inline std::ostream& operator << (std::ostream& out, bitset const& bs) {
  return out << std::string{ bs };
}

template <typename RND>
bitset bitset::random (siz_t size, RND& rnd) {
  bitset bs(size, false);
  std::uniform_int_distribution<bck_t> dist;

  for (siz_t i = 0; i < bs.buckets(); ++i) {
    bs.data(i) = dist(rnd);
  }

  bs.fix_last<false>();
  bs.fix_popcount();
  return bs;
}
