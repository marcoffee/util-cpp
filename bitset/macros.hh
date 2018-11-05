#pragma once

#include "../util_constexpr.hh"

#define EVAL_2(a, b, op, una, i) una(a.bucket(i) op b.bucket(i))

#define OP_BLOCK(out, i) \
  _pop += util::popcount(_bck); \
  out.data(i) = _bck

#define OP_2(a, b, out, op, una) { \
  bitset::siz_t const _lst = a.buckets() - 1; \
  bitset::siz_t _pop = 0; \
  \
  _Pragma("omp simd reduction(+: _pop)") \
  for (bitset::siz_t i = 0; i < _lst; ++i) { \
    bitset::bck_t _bck = EVAL_2(a, b, op, una, i); \
    OP_BLOCK(out, i); \
  } \
  \
  bitset::bck_t _bck = EVAL_2(a, b, op, una, _lst) & a.last_mask(); \
  OP_BLOCK(out, _lst); \
  out.impl_->popcount_ = _pop; \
}

#define POP_2(a, b, out, op, una) { \
  bitset::siz_t const _lst = a.buckets() - 1; \
  \
  _Pragma(TO_STRING(omp simd reduction(+: out))) \
  for (bitset::siz_t i = 0; i < _lst; ++i) { \
    out += util::popcount(EVAL_2(a, b, op, una, i)); \
  } \
  \
  out += util::popcount(EVAL_2(a, b, op, una, _lst) & a.last_mask()); \
}

#define and3(a, b, c) ((a) & (b) & (c))
#define maj(a, b, c) (((a) & (b)) | ((a) & (c)) | ((b) & (c)))
#define ite(a, b, c) (((a) & (b)) | (~(a) & (c)))

#define EVAL_3(a, b, c, op, una, i) una(op(a.bucket(i), b.bucket(i), c.bucket(i)))

#define OP_3(a, b, c, out, op, una) { \
  bitset::siz_t const _lst = a.buckets() - 1; \
  bitset::siz_t _pop = 0; \
  \
  _Pragma("omp simd reduction(+: _pop)") \
  for (bitset::siz_t i = 0; i < _lst; ++i) { \
    bitset::bck_t _bck = EVAL_3(a, b, c, op, una, i); \
    OP_BLOCK(out, i); \
  } \
  \
  bitset::bck_t _bck = EVAL_3(a, b, c, op, una, _lst) & a.last_mask(); \
  OP_BLOCK(out, _lst); \
  out.impl_->popcount_ = _pop; \
}

#define POP_3(a, b, c, out, op, una) { \
  bitset::siz_t const _lst = a.buckets() - 1; \
  \
  _Pragma(TO_STRING(omp simd reduction(+: out))) \
  for (bitset::siz_t i = 0; i < _lst; ++i) { \
    out += util::popcount(EVAL_3(a, b, c, op, una, i)); \
  } \
  \
  out += util::popcount(EVAL_3(a, b, c, op, una, _lst) & a.last_mask()); \
}
