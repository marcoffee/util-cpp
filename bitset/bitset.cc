#include "bitset.hh"
#include "bitset_macros.hh"

void bitset::fill_data (bool zeros) {
  uintmax_t last;
  bitset::bitset_sizes(this->size(), this->_buckets, last);

  if (last) {
    this->_last = (static_cast<bitset::type>(1) << last) - 1;
  } else {
    this->_last = bitset::ones;
  }

  this->alloc_data(zeros);

  if (!zeros) {
    this->fix_popcount();
  }
}

void bitset::alloc_data (bool zeros) {
  this->set_data(
    ts_ptr<uintmax_t>::make(0),
    ts_ptr<bitset::type[]>::make(this->buckets(), zeros)
  );
}

void bitset::copy_meta (bitset const& other) {
  this->_last = other._last;
  this->_size = other._size;
  this->_buckets = other._buckets;
  this->_mask = other._mask;
}

void bitset::set_data (ts_ptr<uintmax_t> const& popcount, ts_ptr<bitset::type[]> const& guard) {
  this->_popcount = popcount;
  this->_guard = guard;
  this->_data = guard.get();
}

void bitset::release_data (void) {
  this->_popcount.release();
  this->_guard.release();
  this->_data = nullptr;
}

bitset& bitset::copy_from (bitset const& other) {
  this->copy_meta(other);
  this->set_data(other._popcount, other._guard);
  return *this;
}

bitset& bitset::move_from (bitset& other) {
  this->copy_meta(other);
  this->set_data(other._popcount, other._guard);
  other.free();
  return *this;
}

void bitset::fix_popcount (void) {
  uintmax_t result = 0;

  #pragma omp simd reduction(+: result)
  for (uintmax_t i = 0; i < this->buckets(); ++i) {
    result += ::popcount(this->_data[i]);
  }

  *this->_popcount = result;
}

bitset& bitset::AND (bitset const& a, bitset const& b, bitset& out) {
  uintmax_t const a_p = a.popcount(), a_s = a.size();
  uintmax_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == 0 || b_p == b_s || a.fast_compare(b) == bitset::compare::equal) {
    out = a;
  } else if (b_p == 0 || a_p == a_s) {
    out = b;
  } else {
    OP_2(a, b, out, &, EMPTY_ARG);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::OR (bitset const& a, bitset const& b, bitset& out) {
  uintmax_t const a_p = a.popcount(), a_s = a.size();
  uintmax_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == a_s || b_p == 0 || a.fast_compare(b) == bitset::compare::equal) {
    out = a;
  } else if (b_p == b_s || a_p == 0) {
    out = b;
  } else {
    OP_2(a, b, out, |, EMPTY_ARG);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::XOR (bitset const& a, bitset const& b, bitset& out) {
  uintmax_t const a_p = a.popcount(), a_s = a.size();
  uintmax_t const b_p = b.popcount(), b_s = b.size();

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

bitset& bitset::NAND (bitset const& a, bitset const& b, bitset& out) {
  uintmax_t const a_p = a.popcount(), a_s = a.size();
  uintmax_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == 0 || b_p == b_s || a.fast_compare(b) == bitset::compare::equal) {
    bitset::NOT(a, out);
  } else if (b_p == 0 || a_p == a_s) {
    bitset::NOT(b, out);
  } else {
    OP_2(a, b, out, &, ~);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::NOR (bitset const& a, bitset const& b, bitset& out) {
  uintmax_t const a_p = a.popcount(), a_s = a.size();
  uintmax_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == a_s || b_p == 0 || a.fast_compare(b) == bitset::compare::equal) {
    bitset::NOT(a, out);
  } else if (b_p == b_s || a_p == 0) {
    bitset::NOT(b, out);
  } else {
    OP_2(a, b, out, |, ~);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::XNOR (bitset const& a, bitset const& b, bitset& out) {
  uintmax_t const a_p = a.popcount(), a_s = a.size();
  uintmax_t const b_p = b.popcount(), b_s = b.size();

  if (a_p == 0) {
    bitset::NOT(b, out);
  } else if (b_p == 0) {
    bitset::NOT(a, out);
  } else if (a_p == a_s) {
    out = b;
  } else if (b_p == b_s) {
    out = a;
  } else if (a.fast_compare(b) == bitset::compare::equal) {
    out.set();
  } else {
    OP_2(a, b, out, ^, ~);
    out.fix_popcount();
  }

  return out;
}

bitset& bitset::MAJ (bitset const& a, bitset const& b, bitset const& c, bitset& out) {
  OP_3(a, b, c, out, maj, EMPTY_ARG);
  out.fix_popcount();
  return out;
}

bitset& bitset::MIN (bitset const& a, bitset const& b, bitset const& c, bitset& out) {
  OP_3(a, b, c, out, maj, ~);
  out.fix_popcount();
  return out;
}

bitset& bitset::ITE (bitset const& a, bitset const& b, bitset const& c, bitset& out) {
  OP_3(a, b, c, out, ite, EMPTY_ARG);
  out.fix_popcount();
  return out;
}

void bitset::free (void) {
  this->_size = 0;
  this->_last = 0;
  this->_buckets = 0;
  this->_mask = 0;
  this->release_data();
}

bitset bitset::copy (void) const {
  bitset bs;
  bs.copy_meta(*this);
  bs.alloc_data(false);

  std::copy_n(this->_data, this->buckets(), bs._data);
  *bs._popcount = this->popcount();
  return bs;
}

bool bitset::iter_bits (uintmax_t& pos, bool& out) const {
  if (pos >= this->size()) {
    return false;
  }

  uintmax_t const ind = bitset::get_ind(pos), bit = bitset::get_bit(pos);
  pos++;

  out = ((this->_data[ind] ^ this->mask()) >> bit) & 1;
  return true;
}

bool operator <  (bitset const& a, bitset const& b) {
  bitset::compare const cmp = a.fast_compare(b);

  if (cmp == bitset::compare::smaller) {
    return true;
  } else if (cmp == bitset::compare::equal || cmp == bitset::compare::bigger) {
    return false;
  }

  if (a.mask() == b.mask()) {
    bool const lesser = std::lexicographical_compare(
      a.data(), a.data() + a.buckets(), b.data(), b.data() + b.buckets()
    );

    if (a.mask()) {
      return !lesser;
    }

    return lesser;
  }

  uintmax_t const last = a.buckets() - 1;

  for (uintmax_t i = 0; i < last; ++i) {
    if ((a._data[i] ^ a.mask()) < (b._data[i] ^ b.mask())) {
      return true;
    }
  }

  return ((a._data[last] ^ a.mask()) & a.last()) < ((b._data[last] ^ b.mask()) & b.last());
}

bool operator == (bitset const& a, bitset const& b) {
  bitset::compare const cmp = a.fast_compare(b);

  if (cmp == bitset::compare::equal) {
    return true;
  } else if (cmp != bitset::compare::unknown) {
    return false;
  }

  if (a.mask() == b.mask()) {
    return std::equal(
      a.data(), a.data() + a.buckets(), b.data(), b.data() + b.buckets()
    );
  }

  uintmax_t const last = a.buckets() - 1;

  for (uintmax_t i = 0; i < last; ++i) {
    if ((a._data[i] ^ a.mask()) != (b._data[i] ^ b.mask())) {
      return false;
    }
  }

  return ((a._data[last] ^ a.mask()) & a.last()) == ((b._data[last] ^ b.mask()) & b.last());
}

std::ostream& operator << (std::ostream& out, bitset const& bs) {
  uintmax_t pos = 0;
  bool bit;

  while (bs.iter_bits(pos, bit)) {
    out << bit;
  }

  return out;
}

bitset* make_bits (bitset* bits, uintmax_t inputs, uintmax_t use_bits) {
  static constexpr std::array<bitset::type, bitset::bits_shift> const lookup(
    pattern_array<bitset::type>::value
  );

  if (use_bits == 0) {
    use_bits = inputs;
  }

  uintmax_t const p2 = UINTMAX_C(1) << use_bits;
  uintmax_t const filled = std::min(inputs, bitset::bits_shift);
  uintmax_t const not_filled = inputs - filled;
  uintmax_t const zeroed = inputs - use_bits;

  for (uintmax_t i = 0; i < inputs; ++i) {
    bits[i] = bitset(p2);
  }

  #pragma omp parallel for schedule(static)
  for (uintmax_t i = zeroed; i < inputs; ++i) {

    if (i < not_filled) {
      uintmax_t const skip = 1 << (not_filled - i - 1), add = skip << 1;
      uintmax_t j = skip, popcount = 0;

      while (j < bits[i].buckets()) {
        bits[i].fill_n(j, skip, bitset::ones, false);
        popcount += bitset::bits * skip;
        j += add;
      }

      *bits[i]._popcount = popcount;
      bits[i].fix_last<true>();

    } else {
      bits[i].fill(lookup[(lookup.size() + i) - inputs]);
    }
  }

  return bits;
}

bitset* make_bits (bitset* bits, uintmax_t inputs, uintmax_t use_bits, mpz_class& status, bool const* keep) {

  mpz_class const max_status = 1_mpz << inputs;

  if (status >= max_status) {
    return nullptr;
  }

  uintmax_t const p2 = UINTMAX_C(1) << use_bits;

  if (!status) {
    status = p2;
    return make_bits(bits, inputs, use_bits);
  }

  uintmax_t last, buckets;

  bitset::bitset_sizes(p2, buckets, last);

  #pragma omp parallel for schedule(guided)
  for (uintmax_t i = 0; i < inputs; ++i) {
    uintmax_t const pos = inputs - i - 1;

    if (keep && keep[pos]) {
      continue;
    }

    mpz_t mpz_status;

    mpz_init2(mpz_status, inputs);
    mpz_set(mpz_status, status.get_mpz_t());

    bits[pos] = bitset(p2);
    bitset::type* data = bits[pos]._data;

    for (uintmax_t j = 0; j < buckets; ++j) {
      for (uintmax_t k = 0; k < bitset::bits; ++k) {
        bitset::type const bit = mpz_tstbit(mpz_status, i);
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
