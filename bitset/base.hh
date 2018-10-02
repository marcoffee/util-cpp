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
  // Bucket type
  using bck_t = uintmax_t;
  // Size type
  using siz_t = uintmax_t;

 private:
  struct impl {
    // Reference type
    using ref_t = uint32_t;

    // Bitset size
    siz_t size_ = 0;
    // Bitset number of set bits
    siz_t popcount_ = 0;
    // Bitset data
    bck_t* data_ = nullptr;
    // Ref count
    std::atomic<ref_t> ref_ = 1;
  };

  // impl object
  impl* impl_ = nullptr;
  // Mask for fast inversion
  bck_t inverted_ = 0;

  void copy_meta (bitset const& ot);
  void release (void);

  bitset& copy_from (bitset& ot);
  bitset& move_from (bitset& ot);

  void fix_popcount (void) {
    // Recalculate bitset popcount
    this->impl_->popcount_ = bitset::popcount_range(this->begin(), this->end());
  }

  // Private getters to simplify code
  bck_t* data (void) { return this->impl_->data_; }
  bck_t& data (siz_t pos) { return this->impl_->data_[pos]; }
  bck_t& back (void) { return this->data(this->buckets() - 1); }

  // Private iterator helpers
  bck_t* begin (void) { return this->impl_->data_; }
  bck_t* end (void) { return this->begin() + this->buckets(); }

 public:
  // Enumeration of possible outcomes from fast_compare
  enum class compare { equal, different, inverted, unknown };

  // Helper members
  constexpr static siz_t const bits = std::numeric_limits<bck_t>::digits;
  constexpr static siz_t const bits_mask = bitset::bits - 1;
  constexpr static siz_t const bits_shift = static_log2_v<bitset::bits>;

  // Gets the bucket index of a position
  constexpr static siz_t get_ind (siz_t pos) {
    return pos >> bitset::bits_shift;
  }

  // Gets the bit index of a position
  constexpr static siz_t get_bit (siz_t pos) {
    return pos & bitset::bits_mask;
  }

  // Counts the number of buckets required for a given size
  constexpr static siz_t count_buckets (siz_t size) {
    return (size + bitset::bits - 1) / bitset::bits;
  }

  // Counts the number of set bits on a given region
  static siz_t popcount_range (bck_t const* begin, bck_t const* end);

  // Generates a random bitset given a random engine
  template <typename RND>
  static bitset random (siz_t size, RND& rnd) {
    bitset bs{ size, false, false };
    siz_t const last = bs.buckets() - 1;
    std::uniform_int_distribution<bck_t> dist;

    for (siz_t i = 0; i < last; ++i) {
      bs.data(i) = dist(rnd);
      bs.impl_->popcount_ += ::popcount(bs.data(i));
    }

    bs.back() = dist(rnd) & bs.last_mask();
    bs.impl_->popcount_ += ::popcount(bs.back());

    return bs;
  }

  // Generates a random bitset
  static bitset random (siz_t size) {
    std::default_random_engine rnd((std::random_device())());
    return bitset::random(size, rnd);
  }

  // Bitwise functions
  static bitset&  NOT (bitset& a, bitset& out) { return (out = ~a); }
  static bitset& WIRE (bitset& a, bitset& out) { return (out = a); }

  static bitset&  AND (bitset a, bitset b, bitset& out);
  static bitset&   OR (bitset a, bitset b, bitset& out);
  static bitset&  XOR (bitset a, bitset b, bitset& out);
  static bitset&  MAJ (bitset a, bitset b, bitset c, bitset& out);
  static bitset&  ITE (bitset a, bitset b, bitset c, bitset& out);

  // Build an array of all possible inputs' combinations
  static void build_combinations (
    bitset* bits, siz_t inputs,
    siz_t use_bits = std::numeric_limits<siz_t>::max(),
    siz_t const* positions = nullptr
  );

  // Default constructor
  bitset (void) {}

  // Constructor
  bitset (siz_t size, bool zeros = true, bool popcount = true)
  : impl_{ new impl } {
    // Counts the number of required buckets
    siz_t const buckets = bitset::count_buckets(size);

    // Updates the values on the impl
    this->impl_->size_ = size;
    this->impl_->popcount_ = 0;
    this->impl_->ref_ = 1;

    if (zeros) {
      // Zeroed bitset
      this->impl_->data_ = new bck_t[buckets]();

    } else {
      // 'Empty' bitset
      this->impl_->data_ = new bck_t[buckets];

      if (popcount) {
        this->fix_popcount();
      }
    }
  }

  // Copy constructor and assignment
  bitset (bitset& ot) { this->copy_from(ot); }
  bitset& operator = (bitset& ot) { return this->copy_from(ot); }

  // Move constructor and assignment
  bitset (bitset&& ot) { this->move_from(ot); }
  bitset& operator = (bitset&& ot) { return this->move_from(ot); }

  // Destructor
  ~bitset (void) { this->free(); }
  void free (void);

  // Generate a copy
  bitset copy (void) const;

  // Gets a bit given its 2d index
  bool get (siz_t ind, siz_t bit) const {
    return (this->bucket(ind) >> bit) & 1;
  }

  // Gets a bit given its 1d index
  bool get (siz_t pos) const {
    return this->get(bitset::get_ind(pos), bitset::get_bit(pos));
  }

  // Sets a bit as true
  template <bool use_mask = true>
  void set (siz_t pos) {
    if constexpr (use_mask) {
      // Does a reset if bitset is inverted
      if (this->inverted()) {
        return this->reset<false>(pos);
      }
    }

    siz_t const ind = bitset::get_ind(pos);
    siz_t const bit = bitset::get_bit(pos);
    bck_t const sel = bck_t{ 1 } << bit;

    // Fixes popcount
    this->impl_->popcount_ += (this->data(ind) & sel) == 0;
    this->data(ind) |= sel;
  }

  // Resets a bit to false
  template <bool use_mask = true>
  void reset (siz_t pos) {
    if constexpr (use_mask) {
      // Does a set if bitset is inverted
      if (this->inverted()) {
        return this->set<false>(pos);
      }
    }

    siz_t const ind = bitset::get_ind(pos);
    siz_t const bit = bitset::get_bit(pos);
    bck_t const sel = bck_t{ 1 } << bit;

    // Fixes popcount
    this->impl_->popcount_ -= (this->data(ind) & sel) != 0;
    this->data(ind) &= ~sel;
  }

  // Flips a bit
  void flip (siz_t pos) {
    siz_t const ind = bitset::get_ind(pos);
    siz_t const bit = bitset::get_bit(pos);
    bck_t const sel = bck_t{ 1 } << bit;

    // Fixed popcount
    this->impl_->popcount_ += (this->data(ind) & sel) ? -1 : 1;
    this->data(ind) ^= sel;
  }

  // Fills bitset with value
  void fill (bck_t value) {
    siz_t const pop = ::popcount(value);
    std::fill(this->begin(), this->end(), value ^ this->inverted());
    this->impl_->popcount_ = this->buckets() * pop;

    // Fixes last bucket
    this->fix_last<true>();
  }

  // Fills a bucket range on bitset with value
  void fill (siz_t begin, siz_t end, bck_t value) {
    // Fixes popcount
    siz_t const new_pop = ::popcount(value) * (end - begin);
    siz_t const old_pop = bitset::popcount_range(
      this->begin() + begin, this->begin() + end
    );

    if (old_pop > new_pop) {
      // Reduce popcount
      this->impl_->popcount_ -= old_pop - new_pop;

    } else {
      // Increase popcount
      this->impl_->popcount_ += new_pop - old_pop;
    }

    value ^= this->inverted();
    std::fill(this->data() + begin, this->data() + end, value);

    // Fixes last bucket
    if (end >= (this->buckets() - 1)) {
      this->fix_last<false>();
    }
  }

  // Fills a bucket range on bitset with value
  void fill_n (siz_t begin, siz_t count, bck_t value) {
    this->fill(begin, begin + count, value);
  }

  // Updates the value of a single bucket
  void set_bucket (siz_t bucket, bck_t value) {
    // Apply inverted
    value ^= this->inverted();

    // Fixes value for last bucket
    if (bucket == (this->buckets() - 1)) {
      value &= this->last_mask();
    }

    // Fixes popcount
    siz_t const old_pop = ::popcount(this->data(bucket));
    this->impl_->popcount_ -= old_pop - ::popcount(value);

    this->data(bucket) = value;
  }

  // Fills the bitset with ones
  void set (void) {
    std::fill(this->begin(), this->end(), ~this->inverted());
    this->fix_last<false>();
    this->impl_->popcount_ = this->inverted() ? 0 : this->size();
  }

  // Fills the bitset with zeros
  void reset (void) {
    std::fill(this->begin(), this->end(), this->inverted());
    this->fix_last<false>();
    this->impl_->popcount_ = this->inverted() ? this->size() : 0;
  }

  // Fixes the last bucket
  template <bool pop>
  void fix_last (void) {
    // If it fits, it does not require fixing
    if (!this->fit()) {
      return;
    }

    bck_t& back = this->data(this->buckets() - 1);
    bck_t const new_back = back & this->last_mask();

    if constexpr (pop) {
      // Updates popcount difference
      this->impl_->popcount_ -= ::popcount(back) - ::popcount(new_back);
    }

    back = new_back;
  }

  // Shallow test if two bitsets are the same
  bool is (bitset const& b) const {
    return this->impl_ == b.impl_ and this->inverted() == b.inverted();
  }

  // Makes a fast comparison (constant time) between two bitsets
  compare fast_compare (bitset const& b) const {
    // Invalid bitsets always are different
    if (!this->valid() or !b.valid()) {
      return compare::different;
    }

    // Gets popcounts
    siz_t const a_p = this->popcount();
    siz_t const b_p = b.popcount();

    // If they differ on size or popcount, they are different
    if (this->size() > b.size() or b.size() > this->size() or a_p != b_p) {
      return compare::different;
    }

    // If they are the same or they are all set or reset, they are the same
    if (this->is(b) or (a_p == b_p and (a_p == this->size() or a_p == 0))) {
      return compare::equal;
    }

    // If they have the same data, but with different masks, they are inverted
    if (this->impl_ == b.impl_ and this->inverted() != b.inverted()) {
      return compare::inverted;
    }

    // The function could not find a result on constant time
    return compare::unknown;
  }

  // Brackets operator
  bool operator [] (siz_t pos) const { return this->get(pos); }

  // Bitwise or
  bitset operator |  (bitset ot) {
    bitset out;
    return bitset::OR(*this, ot, out);
  }

  // Bitwise and
  bitset operator &  (bitset ot) {
    bitset out;
    return bitset::AND(*this, ot, out);
  }

  // Bitwise xor
  bitset operator ^  (bitset ot) {
    bitset out;
    return bitset::XOR(*this, ot, out);
  }

  // Bitwise or in place
  bitset& operator |= (bitset ot) {
    return bitset::OR(*this, ot, *this);
  }

  // Bitwise and in place
  bitset& operator &= (bitset ot) {
    return bitset::AND(*this, ot, *this);
  }

  // Bitwise xor in place
  bitset& operator ^= (bitset ot) {
    return bitset::XOR(*this, ot, *this);
  }

  // Bitwise not
  bitset operator ~  (void) {
    bitset bs = *this;
    return bs.flip();
  }

  // Bitwise not in place
  bitset& flip (void) {
    this->inverted_ = ~this->inverted_;
    return *this;
  }

  // Other operators
  bool operator == (bitset const& ot) const;
  explicit operator std::string (void) const;

  // Getters
  siz_t size (void) const { return this->impl_->size_; }
  bck_t const* data (void) const { return this->impl_->data_; }
  bck_t data (siz_t pos) const { return this->impl_->data_[pos]; }
  bck_t bucket (siz_t pos) const { return this->data(pos) ^ this->inverted(); }
  bck_t inverted (void) const { return this->inverted_; }
  bool fit (void) const { return !(this->size() & bitset::bits_mask); }
  bool valid (void) const { return this->impl_ != nullptr; }

  // Number of buckets available
  siz_t buckets (void) const {
    return (this->size() + bitset::bits - 1) / bitset::bits;
  }

  // Mask of the last position on the bitset
  bck_t last_mask (void) const {
    constexpr auto zero = bck_t{ 0 };
    constexpr auto one = bck_t{ 1 };

    // Calculates the last bucket size
    bck_t const last = this->get_bit(this->size());
    return ((one << last) - one) | ((zero - (last == zero)) & ~zero);
  }

  // Number of bits on the last bucket
  siz_t last_bits (void) const {
    return this->fit() ? bitset::bits : this->get_bit(this->size());
  }

  // Number of bits set to true
  siz_t const popcount (void) const {
    if (this->inverted()) {
      return this->size() - this->impl_->popcount_;
    }

    return this->impl_->popcount_;
  }
};

// Bitset stream operator
inline std::ostream& operator << (std::ostream& out, bitset const& bs) {
  return out << std::string{ bs };
}
