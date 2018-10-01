#pragma once

#define OP_1(a, out, una)\
_Pragma("omp simd")\
for (bitset::siz_t i = 0; i < a.buckets(); ++i) {\
  out.data(i) = una(a.data(i));\
}\
out.fix_last<true>();

#define OP_2(a, b, out, op, una)\
_Pragma("omp simd")\
for (bitset::siz_t i = 0; i < a.buckets(); ++i) {\
  out.data(i) = una((a.data(i) ^ a.mask()) op (b.data(i) ^ b.mask()));\
}\
out.fix_last<true>();

#define maj(a, b, c) (((a) &  (b)) | ((a) & (c)) | ((b) & (c)))
#define ite(a, b, c) (((a) & (b)) | (~(a) & (c)))

#define OP_3(a, b, c, out, op, una)\
_Pragma("omp simd")\
for (bitset::siz_t i = 0; i < a.buckets(); ++i) {\
  out.data(i) = una(op(a.data(i) ^ a.mask(), b.data(i) ^ b.mask(), c.data(i) ^ c.mask()));\
}\
out.fix_last<true>();
