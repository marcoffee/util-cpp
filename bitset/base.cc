#include <iomanip>
#include "base.hh"
#include "macros.hh"

void bitset::copy_meta (bitset const& ot) {
  this->mask_ = ot.mask_;
}

void bitset::release (void) {
  this->impl_ = nullptr;
  this->mask_ = 0;
}

void bitset::free (void) {
  if (this->impl_ != nullptr) {
    if (!--this->impl_->ref_) {
      delete[] this->impl_->data_;
      delete this->impl_;
    }

    this->release();
  }
}

bitset& bitset::copy_from (bitset& ot) {
  if (this != &ot) {
    this->free();
    this->copy_meta(ot);
    this->impl_ = ot.impl_;
    ++this->impl_->ref_;
  }

  return *this;
}

bitset& bitset::move_from (bitset& ot) {
  if (this != &ot) {
    this->free();
    this->copy_meta(ot);
    this->impl_ = ot.impl_;
    ot.release();
  }

  return *this;
}

void bitset::fix_popcount (void) {
  siz_t result = 0;

  #pragma omp simd reduction(+: result)
  for (siz_t i = 0; i < this->buckets(); ++i) {
    result += ::popcount(this->data(i));
  }

  this->impl_->popcount_ = result;
}

bitset& bitset::AND (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == 0 or b_p == b_s or a.fast_compare(b) == bitset::compare::equal) {
    out = a;
  } else if (b_p == 0 or a_p == a_s) {
    out = b;
  } else {
    OP_2(a, b, out, &, EMPTY_ARG);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::OR (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == a_s or b_p == 0 or a.fast_compare(b) == bitset::compare::equal) {
    out = a;
  } else if (b_p == b_s or a_p == 0) {
    out = b;
  } else {
    OP_2(a, b, out, |, EMPTY_ARG);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::XOR (bitset a, bitset b, bitset& out) {
  siz_t const a_p = a.popcount(), a_s = a.size();
  siz_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == 0) {
    out = b;
  } else if (b_p == 0) {
    out = a;
  } else if (a_p == a_s) {
    bitset::NOT(b, out);
  } else if (b_p == b_s) {
    bitset::NOT(a, out);
  } else if (a.fast_compare(b) == bitset::compare::equal) {
    out.reset();
  } else {
    OP_2(a, b, out, ^, EMPTY_ARG);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::MAJ (bitset a, bitset b, bitset c, bitset& out) {
  OP_3(a, b, c, out, maj, EMPTY_ARG);
  out.fix_popcount();
  return out;
}

bitset& bitset::ITE (bitset a, bitset b, bitset c, bitset& out) {
  OP_3(a, b, c, out, ite, EMPTY_ARG);
  out.fix_popcount();
  return out;
}

bitset bitset::copy (void) const {
  bitset bs(this->size(), false);
  std::copy_n(this->data(), this->buckets(), bs.data());
  bs.impl_->popcount_ = this->popcount();
  return bs;
}

bool bitset::operator == (bitset const& ot) const {
  bitset::compare const cmp = this->fast_compare(ot);

  if (cmp == bitset::compare::equal) {
    return true;

  } else if (cmp != bitset::compare::unknown) {
    return false;
  }

  if (this->mask() == ot.mask()) {
    return std::equal(
      this->data(), this->data() + this->buckets(),
      ot.data(), ot.data() + ot.buckets()
    );
  }

  siz_t const last = this->buckets() - 1;

  for (siz_t i = 0; i < last; ++i) {
    if ((this->data(i) ^ this->mask()) != (ot.data(i) ^ ot.mask())) {
      return false;
    }
  }

  return (
    ((this->data(last) ^ this->mask()) & this->last_mask()) ==
    ((ot.data(last) ^ ot.mask()) & ot.last_mask())
  );
}

bitset::operator std::string (void) const {
  using bck_t = bitset::bck_t;
  using siz_t = bitset::siz_t;

  if (!this->buckets()) {
    return "0";
  }

  std::string result;
  result.reserve(this->size());

  siz_t const last_pos = this->buckets() - 1;

  for (siz_t i = 0; i < last_pos; ++i) {
    constexpr siz_t wid = bitset::bits >> 2;
    bck_t const bck = this->data(i) ^ this->mask();
    std::ostringstream ss;
    ss << std::hex << std::setw(wid) << std::setfill('0') << std::right << bck;

    std::string const& str = ss.str();
    std::copy(str.rbegin(), str.rend(), std::back_inserter(result));
  }

  std::ostringstream ss;
  bck_t const bck = (this->data(last_pos) ^ this->mask()) & this->last_mask();
  siz_t const wid = (this->last_bits() + 3) / 4;
  ss << std::hex<< std::setw(wid) << std::setfill('0') << std::right << bck;

  std::cout << wid << std::endl;

  std::string const& str = ss.str();
  std::copy(str.rbegin(), str.rend(), std::back_inserter(result));

  return result;
}

bitset* bitset::build_combinations (
  bitset* bits, siz_t inputs, siz_t use_bits, siz_t const* positions
) {
  static constexpr std::array<bitset::bck_t, bitset::bits_shift> const lookup{
    pattern_array<bitset::bck_t>::value
  };

  if (use_bits == 0) {
    use_bits = inputs;
  }

  siz_t const p2 = bitset::siz_t{ 1 } << use_bits;
  siz_t const filled = std::min(inputs, bitset::bits_shift);
  siz_t const not_filled = inputs - filled;
  siz_t const zeroed = inputs - use_bits;

  for (siz_t i = 0; i < inputs; ++i) {
    bits[positions ? positions[i] : i] = bitset(p2);
  }

  #pragma omp parallel for schedule(static)
  for (siz_t i = zeroed; i < inputs; ++i) {
    bitset& bs = bits[positions ? positions[i] : i];

    if (i < not_filled) {
      siz_t const skip = 1 << (not_filled - i - 1), add = skip << 1;
      siz_t j = skip, popcount = 0;

      while (j < bs.buckets()) {
        bs.fill_n(j, skip, bitset::ones, false);
        popcount += bitset::bits * skip;
        j += add;
      }

      bs.impl_->popcount_ = popcount;
      bs.fix_last<true>();

    } else {
      bs.fill(lookup[(lookup.size() + i) - inputs]);
    }
  }

  return bits;
}

bitset* bitset::build_combinations (
  bitset* bits, siz_t inputs, siz_t use_bits, mpz_class& status,
  bool const* keep
) {
  mpz_class const max_status = 1_mpz << inputs;

  if (status >= max_status) {
    return nullptr;
  }

  siz_t const p2 = siz_t{ 1 } << use_bits;

  if (!status) {
    status = p2;
    return bitset::build_combinations(bits, inputs, use_bits);
  }

  siz_t last, buckets;

  bitset::bitset_sizes(p2, buckets, last);

  #pragma omp parallel for schedule(guided)
  for (siz_t i = 0; i < inputs; ++i) {
    siz_t const pos = inputs - i - 1;

    if (keep and keep[pos]) {
      continue;
    }

    mpz_t mpz_status;

    mpz_init2(mpz_status, inputs);
    mpz_set(mpz_status, status.get_mpz_t());

    bits[pos] = bitset(p2);
    bitset::bck_t* data = bits[pos].data();

    for (siz_t j = 0; j < buckets; ++j) {
      for (siz_t k = 0; k < bitset::bits; ++k) {
        bitset::bck_t const bit = mpz_tstbit(mpz_status, i);
        data[j] |= bit << k;
        mpz_add_ui(mpz_status, mpz_status, 1);
      }
    }

    bits[pos].fix_last<false>();
    bits[pos].fix_popcount();

    mpz_clear(mpz_status);
  }

  status += p2;

  return bits;
}
