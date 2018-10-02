#pragma once

#define OP_1(a, out, una) \
_Pragma("omp simd") \
for (bitset::siz_t i = 0; i < a.buckets(); ++i) { \
  out.data(i) = una(a.data(i)); \
}

#define OP_2(a, b, out, op, una) \
_Pragma("omp simd") \
for (bitset::siz_t i = 0; i < a.buckets(); ++i) { \
  out.data(i) = una(a.bucket(i) op b.bucket(i)); \
}

#define maj(a, b, c) (((a) & (b)) | ((a) & (c)) | ((b) & (c)))
#define ite(a, b, c) (((a) & (b)) | (~(a) & (c)))

#define OP_3(a, b, c, out, op, una) \
_Pragma("omp simd") \
for (bitset::siz_t i = 0; i < a.buckets(); ++i) { \
  out.data(i) = una(op(a.bucket(i), b.bucket(i), c.bucket(i))); \
}
