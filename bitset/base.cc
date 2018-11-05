#include <iomanip>
#include "base.hh"
#include "macros.hh"

// Bucket type
using bck_t = bitset::bck_t;
// Size type
using siz_t = bitset::siz_t;

// Copy metadata from other bitset
void bitset::copy_meta (bitset const& ot) {
  this->inverted_ = ot.inverted_;
}

// Release bitset storage
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

    if (this->impl_) {
      ++this->impl_->ref_;
    }
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
siz_t bitset::popcount_range (bck_t const* begin, bck_t const* end) {
  siz_t result = 0;

  #pragma omp simd reduction(+: result)
  for (bck_t const* i = begin; i < end; ++i) {
    result += util::popcount(*i);
  }

  return result;
}

// Build an array of all possible inputs' combinations
void bitset::build_combinations (bitset* bsets, siz_t inputs) {
  // Build a lookup table
  static constexpr std::array const lookup{ pattern_array_v<bck_t> };

  siz_t const p2 = siz_t{ 1 } << inputs;
  siz_t const filled = std::min(inputs, bitset::bits_shift);
  siz_t const not_filled = inputs - filled;

  #pragma omp parallel for schedule(dynamic)
  for (siz_t i = 0; i < inputs; ++i) {
    // Allocate bitset
    bitset& bs = bsets[i];
    bs = bitset{ p2, false, false };

    // Fill bitsets where the pattern will be smaller than bitset::bits bits
    if (i >= not_filled) {
      bs.fill(lookup[(lookup.size() + i) - inputs]);
      continue;
    }

    // Fill the bitset with zero
    bs.reset();

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
  if (a_p == 0 or b_p == b_s or a.fast_compare(b) == compare::equal) {
    out = std::move(a);

  // If b is all zeroes or a is all ones
  } else if (b_p == 0 or a_p == a_s) {
    out = std::move(b);

  // Evaluate the AND
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size(), false, false };
    }

    // Remove inversion flag
    out.inverted_ = 0;
    OP_2(a, b, out, &, EMPTY_ARG);
  }

  return out;
}

// Bitwise OR of two bitsets
// __attribute__((target("no-sse")))
bitset& bitset::OR (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  // If a is all ones, b is all zeroes, or a is equal to b
  if (a_p == a_s or b_p == 0 or a.fast_compare(b) == compare::equal) {
    out = std::move(a);

  // if b is all ones or a is all zeroes
  } else if (b_p == b_s or a_p == 0) {
    out = std::move(b);

  // Evaluate the OR
  } else {
    if (!out.valid() or out.size() < a.size()) {
      out = bitset{ a.size(), false, false };
    }

    // Remove inversion flag
    out.inverted_ = 0;
    OP_2(a, b, out, |, EMPTY_ARG);
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
      out = bitset{ a.size(), false, false };
    }

    // Remove inversion flag
    out.inverted_ = 0;
    OP_2(a, b, out, ^, EMPTY_ARG);
  }

  return out;
}

// Bitwise MAJ of three bitsets
// __attribute__((target("no-sse")))
bitset& bitset::MAJ (bitset a, bitset b, bitset c, bitset& out) {
  constexpr auto equal = compare::equal;
  constexpr auto inverted = compare::inverted;

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
      out = bitset{ a.size(), false, false };
    }

    // Remove inversion flag
    out.inverted_ = 0;
    OP_3(a, b, c, out, maj, EMPTY_ARG);
  }
  return out;
}

// Memory aware popcount of bitwise AND between two bitsets
// __attribute__((target("no-sse")))
siz_t bitset::AND_popcount (bitset a, bitset b) {
  siz_t out = 0;
  POP_2(a, b, out, &, EMPTY_ARG);
  return out;
}

// Memory aware popcount of bitwise OR between two bitsets
// __attribute__((target("no-sse")))
siz_t bitset::OR_popcount (bitset a, bitset b) {
  siz_t out = 0;
  POP_2(a, b, out, |, EMPTY_ARG);
  return out;
}

// Memory aware popcount of bitwise XOR between two bitsets
// __attribute__((target("no-sse")))
siz_t bitset::XOR_popcount (bitset a, bitset b) {
  siz_t out = 0;
  POP_2(a, b, out, ^, EMPTY_ARG);
  return out;
}

// Memory aware popcount of bitwise MAJ between three bitsets
// __attribute__((target("no-sse")))
siz_t bitset::MAJ_popcount (bitset a, bitset b, bitset c) {
  siz_t out = 0;
  POP_3(a, b, c, out, maj, EMPTY_ARG);
  return out;
}

// Memory aware popcount of bitwise AND between three bitsets
// __attribute__((target("no-sse")))
siz_t bitset::AND3_popcount (bitset a, bitset b, bitset c) {
  siz_t out = 0;
  POP_3(a, b, c, out, and3, EMPTY_ARG);
  return out;
}

// Generate a copy
bitset bitset::copy (void) const {
  bitset bs{ this->size(), false, false };
  bs.copy_meta(*this);
  bs.impl_->popcount_ = this->impl_->popcount_;
  std::copy_n(this->data(), this->buckets(), bs.data());
  return bs;
}

// Compare two bitsets
bool bitset::operator == (bitset const& ot) const {
  // Try fast comparison
  compare const cmp = this->fast_compare(ot);

  if (cmp == compare::equal) {
    return true;

  } else if (cmp != compare::unknown) {
    return false;
  }

  siz_t const last_pos = this->buckets() - 1;
  bool eq = true;

  // Full equality test
  #pragma omp simd reduction(&&: eq)
  for (siz_t i = 0; i < last_pos; ++i) {
    eq = eq and this->bucket(i) != ot.bucket(i);
  }

  return eq and (
    (this->bucket(last_pos) & this->last_mask()) ==
    (ot.bucket(last_pos) & ot.last_mask())
  );
}

// Converts the bitset to a hex string
bitset::operator std::string (void) const {
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

  // Copy inverted to string
  std::string const& str = ss.str();
  std::copy(str.rbegin(), str.rend(), std::back_inserter(result));

  return result;
}
