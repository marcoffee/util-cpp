#pragma once

#include <iterator>

#define EMPTY_ARG
#define COMMA ,

#ifndef RST_LINE
  #define RST_LINE "\r\033[K"
#endif

#ifndef WHITESPACE
  #define WHITESPACE " \f\n\r\t\v"
#endif

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#ifdef _OPENMP
  #define IF_OMP(v) _Pragma(TO_STRING(omp v))
#else
  #define IF_OMP(v)
#endif

#if __cpp_if_constexpr >= 201606
  #define if_static(x) if constexpr(x)
#else
  #define if_static(x) if (x)
#endif

#define do_every(every, max, at) \
if ((every) != 0 && ((((at) + (every) >= (max))) || (((at) % (every)) == 0)))

#define macro_ternary_(t, f) f
#define macro_ternary_0(t, f) f
#define macro_ternary_false(t, f) f
#define macro_ternary_1(t, f) t
#define macro_ternary_true(t, f) t
#define macro_ternary(cond, t, f) macro_ternary_ ## cond (t, f)

#define expand_mutable_iterators(param, it, _b, _e, suf, ce) \
  macro_ternary(ce, constexpr, inline) it suf ## begin param { return (_b); } \
  macro_ternary(ce, constexpr, inline) it suf ## end param { return (_e); }

#define expand_const_iterators(param, cit, _b, _e, suf, ce) \
  macro_ternary(ce, constexpr, inline) cit suf ## begin param const { return (_b); } \
  macro_ternary(ce, constexpr, inline) cit suf ## end param const { return (_e); } \
  macro_ternary(ce, constexpr, inline) cit c ## suf ## begin param const { return (_b); } \
  macro_ternary(ce, constexpr, inline) cit c ## suf ## end param const { return (_e); }

#define expand_mutable_reverse_iterators(param, it, _b, _e, suf, ce) \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<it> r ## suf ## begin param { return std::make_reverse_iterator<it>(_e); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<it> r ## suf ## end param { return std::make_reverse_iterator<it>(_b); }

#define expand_const_reverse_iterators(param, cit, _b, _e, suf, ce) \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> r ## suf ## begin param const { return std::make_reverse_iterator<cit>(_e); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> r ## suf ## end param const { return std::make_reverse_iterator<cit>(_b); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> cr ## suf ## begin param const { return std::make_reverse_iterator<cit>(_e); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> cr ## suf ## end param const { return std::make_reverse_iterator<cit>(_b); }

#define expand_iterators(param, it, cit, _b, _e, suf, ce) \
  expand_mutable_iterators(param, it, _b, _e, suf, ce) \
  expand_const_iterators(param, cit, _b, _e, suf, ce)

#define expand_reverse_iterators(param, it, cit, _b, _e, suf, ce) \
  expand_mutable_reverse_iterators(param, it, _b, _e, suf, ce) \
  expand_const_reverse_iterators(param, cit, _b, _e, suf, ce)

#define expand_all_iterators(param, it, cit, _b, _e, suf, ce) \
  expand_iterators(param, it, cit, _b, _e, suf, ce) \
  expand_reverse_iterators(param, it, cit, _b, _e, suf, ce)

#define expand_all_mutable_iterators(param, it, _b, _e, suf, ce) \
  expand_mutable_iterators(param, it, _b, _e, suf, ce) \
  expand_mutable_reverse_iterators(param, it, _b, _e, suf, ce)

#define expand_all_const_iterators(param, cit, _b, _e, suf, ce) \
  expand_const_iterators(param, cit, _b, _e, suf, ce) \
  expand_const_reverse_iterators(param, cit, _b, _e, suf, ce)
