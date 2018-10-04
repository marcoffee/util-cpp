#include <iomanip>
#include "base.hh"
#include "macros.hh"

// Copy metadata from other bitset
void bitset::copy_meta (bitset const& ot) {
  this->inverted_ = ot.inverted_;
}

// Release bitset values
void bitset::release (void) {
  this->impl_ = nullptr;
  this->inverted_ = 0;
}

// Free bitset memory
void bitset::free (void) {
  if (this->valid()) {

    // Free impl only if this was the last reference
    if (!--this->impl_->ref_) {
      delete[] this->impl_->data_;
      delete this->impl_;
    }

    // Release data
    this->release();
  }
}

// Copy from another bitset
bitset& bitset::copy_from (bitset& ot) {
  if (this != &ot) {
    this->free();
    this->copy_meta(ot);
    this->impl_ = ot.impl_;
    ++this->impl_->ref_;
  }

  return *this;
}

// Move from another bitset
bitset& bitset::move_from (bitset& ot) {
  if (this != &ot) {
    this->free();
    this->copy_meta(ot);
    this->impl_ = ot.impl_;
    ot.release();
  }

  return *this;
}

// Counts the number of set bits on a given region
bitset::siz_t bitset::popcount_range (bck_t const* begin, bck_t const* end) {
  siz_t result = 0;

  #pragma omp simd reduction(+: result)
  for (bck_t const* i = begin; i < end; ++i) {
    result += ::popcount(*i);
  }

  return result;
}

// Build an array of all possible inputs' combinations
void bitset::build_combinations (
  bitset* bits, siz_t inputs, siz_t use_bits, siz_t const* positions
) {
  // Build a lookup table
  static constexpr std::array const lookup{ pattern_array_v<bitset::bck_t> };

  // Number of bits to use
  use_bits = std::min(use_bits, inputs);

  siz_t const p2 = bitset::siz_t{ 1 } << use_bits;
  siz_t const filled = std::min(inputs, bitset::bits_shift);
  siz_t const not_filled = inputs - filled;
  siz_t const zeroed = inputs - use_bits;

  #pragma omp parallel for schedule(dynamic)
  for (siz_t i = 0; i < inputs; ++i) {
    // Build bitset
    bitset& bs = bits[positions ? positions[i] : i];
    bs = bitset{ p2 };

    // Skip bitsets that will remain zeroed because of the number of bits used
    if (i < zeroed) {
      continue;
    }

    // Fill bitsets where the pattern will be smaller than 64 bits
    if (i >= not_filled) {
      bs.fill(lookup[(lookup.size() + i) - inputs]);
      continue;
    }

    // Otherwise, fill the buckets according to its position
    siz_t const contig = 1 << (not_filled - i - 1);
    siz_t const next = contig * 2;
    siz_t fills = 0;

    for (bck_t* it = bs.begin() + contig; it < bs.end(); it += next) {
      std::fill(it, it + contig, ~bck_t{ 0 });
      ++fills;
    }

    bs.impl_->popcount_ = bitset::bits * contig * fills;
    bs.fix_last<true>();
  }
}

// Bitwise AND of two bitsets
// __attribute__((target("no-sse")))
bitset& bitset::AND (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  // If a is all zeroes, b is all ones, or a is equal to b
  if (a_p == 0 or b_p == b_s or a.fast_compare(b) == bitset::compare::equal) {
    out = std::move(a);

  // If b is all zeroes or a is all ones
  } else if (b_p == 0 or a_p == a_s) {
    out = std::move(b);

  // Evaluate the AND
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size() };
    }

    OP_2(a, b, out, &, EMPTY_ARG);
    out.fix_last<false>();
    out.fix_popcount();
  }

  return out;
}

// Bitwise OR of two bitsets
// __attribute__((target("no-sse")))
bitset& bitset::OR (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  // If a is all ones, b is all zeroes, or a is equal to b
  if (a_p == a_s or b_p == 0 or a.fast_compare(b) == bitset::compare::equal) {
    out = std::move(a);

  // if b is all ones or a is all zeroes
  } else if (b_p == b_s or a_p == 0) {
    out = std::move(b);

  // Evaluate the OR
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size() };
    }

    OP_2(a, b, out, |, EMPTY_ARG);
    out.fix_last<false>();
    out.fix_popcount();
  }

  return out;
}

// Bitwise XOR of two bitsets
// __attribute__((target("no-sse")))
bitset& bitset::XOR (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  // If a is all zeroes
  if (a_p == 0) {
    out = std::move(b);

  // If b is all zeroes
  } else if (b_p == 0) {
    out = std::move(a);

  // If a is all ones
  } else if (a_p == a_s) {
    bitset::NOT(b, out);

  // If b is all ones
  } else if (b_p == b_s) {
    bitset::NOT(a, out);

  // Evaluate the XOR
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size() };
    }

    OP_2(a, b, out, ^, EMPTY_ARG);
    out.fix_last<false>();
    out.fix_popcount();
  }

  return out;
}

// Bitwise MAJ of three bitsets
// __attribute__((target("no-sse")))
bitset& bitset::MAJ (bitset a, bitset b, bitset c, bitset& out) {
  constexpr auto equal = bitset::compare::equal;
  constexpr auto inverted = bitset::compare::inverted;

  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();
  siz_t const c_p = c.popcount(), c_s = c.size();
  compare a_cmp_b = a.fast_compare(b);
  compare a_cmp_c = a.fast_compare(c);
  compare b_cmp_c = b.fast_compare(c);

  // If a is equal to b, a is equal to c, or b is ~c
  if (a_cmp_b == equal or a_cmp_c == equal or b_cmp_c == inverted) {
    out = std::move(a);

  // If b is equal to c, or a is ~c
  } else if (b_cmp_c == equal or a_cmp_c == inverted) {
    out = std::move(b);

  // If a is ~b
  } else if (a_cmp_b == inverted) {
    out = std::move(c);

  // If a is all zeroes
  } else if (a_p == 0) {
    bitset::AND(b, c, out);

  // If b is all zeroes
  } else if (b_p == 0) {
    bitset::AND(a, c, out);

  // If c is all zeroes
  } else if (c_p == 0) {
    bitset::AND(a, b, out);

  // If a is all ones
  } else if (a_p == a_s) {
    bitset::OR(b, c, out);

  // If b is all ones
  } else if (b_p == b_s) {
    bitset::OR(a, c, out);

  // If c is all ones
  } else if (c_p == c_s) {
    bitset::OR(a, b, out);

  // Evaluate the MAJ
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size() };
    }

    OP_3(a, b, c, out, maj, EMPTY_ARG);
    out.fix_last<false>();
    out.fix_popcount();
  }
  return out;
}

// Bitwise ITE of three bitsets
// __attribute__((target("no-sse")))
bitset& bitset::ITE (bitset a, bitset b, bitset c, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();

  // If a is all zeroes or b is equal to c
  if (a_p == 0 or b.fast_compare(c) == bitset::compare::equal) {
    out = std::move(c);

  // If a is all ones
  } else if (a_p == a_s) {
    out = std::move(b);

  // Evaluate the ITE
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size() };
    }

    OP_3(a, b, c, out, ite, EMPTY_ARG);
    out.fix_last<false>();
    out.fix_popcount();
  }

  return out;
}

// Generate a copy
bitset bitset::copy (void) const {
  bitset bs{ this->size(), false, false };
  std::copy_n(this->data(), this->buckets(), bs.data());
  bs.impl_->popcount_ = this->popcount();
  return bs;
}

// Compare two bitsets
bool bitset::operator == (bitset const& ot) const {
  // Try fast comparison
  bitset::compare const cmp = this->fast_compare(ot);

  if (cmp == bitset::compare::equal) {
    return true;

  } else if (cmp != bitset::compare::unknown) {
    return false;
  }

  // Non-masked equality test
  if (this->inverted() == ot.inverted()) {
    return std::equal(
      this->data(), this->data() + this->buckets(),
      ot.data(), ot.data() + ot.buckets()
    );
  }

  // Masked equality test
  siz_t const last_pos = this->buckets() - 1;

  for (siz_t i = 0; i < last_pos; ++i) {
    if (this->bucket(i) != ot.bucket(i)) {
      return false;
    }
  }

  return (
    (this->bucket(last_pos) & this->last_mask()) ==
    (ot.bucket(last_pos) & ot.last_mask())
  );
}

// Converts the bitset to a hex string
bitset::operator std::string (void) const {
  using bck_t = bitset::bck_t;
  using siz_t = bitset::siz_t;

  // Empty bitset
  if (!(this->valid() and this->buckets())) {
    return "0";
  }

  std::string result;
  result.reserve((this->size() + 3) / 4);

  siz_t const last_pos = this->buckets() - 1;

  // Convert each bucket, except the last
  for (siz_t i = 0; i < last_pos; ++i) {
    constexpr siz_t wid = bitset::bits / 4;
    bck_t const bck = this->bucket(i);
    std::ostringstream ss;
    ss << std::hex << std::setw(wid) << std::setfill('0') << std::right << bck;

    // Copy inverted to string
    std::string const& str = ss.str();
    std::copy(str.rbegin(), str.rend(), std::back_inserter(result));
  }

  // Convert the last bucket considering the mask
  std::ostringstream ss;
  bck_t const bck = this->bucket(last_pos) & this->last_mask();
  siz_t const wid = (this->last_bits() + 3) / 4;
  ss << std::hex << std::setw(wid) << std::setfill('0') << std::right << bck;

  std::cout << wid << std::endl;

  // Copy inverted to string
  std::string const& str = ss.str();
  std::copy(str.rbegin(), str.rend(), std::back_inserter(result));

  return result;
}
