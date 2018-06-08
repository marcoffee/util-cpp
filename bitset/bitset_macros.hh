#pragma once

#define OP_1(a, out, una)\
_Pragma("omp simd")\
for (uint i = 0; i < a.buckets(); ++i) {\
  out._data[i] = una(a._data[i]);\
}\
out.fix_last<true>();

#define OP_2(a, b, out, op, una)\
_Pragma("omp simd")\
for (uint i = 0; i < a.buckets(); ++i) {\
  out._data[i] = una((a._data[i] ^ a._mask) op (b._data[i] ^ b._mask));\
}\
out.fix_last<true>();

#define maj(a, b, c) (((a) &  (b)) | ((a) & (c)) | ((b) & (c)))
#define ite(a, b, c) (((a) & (b)) | (~(a) & (c)))

#define OP_3(a, b, c, out, op, una)\
_Pragma("omp simd")\
for (uint i = 0; i < a.buckets(); ++i) {\
  out._data[i] = una(op(a._data[i] ^ a._mask, b._data[i] ^ b._mask, c._data[i] ^ c._mask));\
}\
out.fix_last<true>();
