#pragma once

#include <iterator>

// An empty macro argument
#define EMPTY_ARG

// A comma for macro expansion
#define COMMA ,

#ifndef RST_LINE
  // Clear terminal line
  #define RST_LINE "\r\033[K"
#endif

#ifndef WHITESPACE
  // Whitespace characters
  #define WHITESPACE " \f\n\r\t\v"
#endif

// Convert parameter to string
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

// Expands to omp pragma if available
#ifdef _OPENMP
  #define IF_OMP(v) _Pragma(TO_STRING(omp v))
#else
  #define IF_OMP(v)
#endif

// Repeats an action inside a loop
#define do_every(start, every, max, at) \
if ((every) != 0 and ( \
  ((at) == (start)) or \
  (((max) != 0) and ((at) + (every) >= (max))) or \
  (((at) % (every)) == 0) \
))

// Ternary operators
#define macro_ternary_(t, f) f
#define macro_ternary_0(t, f) f
#define macro_ternary_false(t, f) f
#define macro_ternary_1(t, f) t
#define macro_ternary_true(t, f) t
#define macro_ternary(cond, t, f) macro_ternary_ ## cond (t, f)

// Auto create iterators
#define expand_mutable_iterators(param, it, bf, ef, suf, ce) \
  macro_ternary(ce, constexpr, inline) it suf ## begin param { return (bf); } \
  macro_ternary(ce, constexpr, inline) it suf ## end param { return (ef); }

#define expand_const_iterators(param, cit, bf, ef, suf, ce) \
  macro_ternary(ce, constexpr, inline) cit suf ## begin param const { return (bf); } \
  macro_ternary(ce, constexpr, inline) cit suf ## end param const { return (ef); } \
  macro_ternary(ce, constexpr, inline) cit c ## suf ## begin param const { return (bf); } \
  macro_ternary(ce, constexpr, inline) cit c ## suf ## end param const { return (ef); }

#define expand_mutable_reverse_iterators(param, it, bf, ef, suf, ce) \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<it> r ## suf ## begin param { return std::make_reverse_iterator<it>(ef); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<it> r ## suf ## end param { return std::make_reverse_iterator<it>(bf); }

#define expand_const_reverse_iterators(param, cit, bf, ef, suf, ce) \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> r ## suf ## begin param const { return std::make_reverse_iterator<cit>(ef); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> r ## suf ## end param const { return std::make_reverse_iterator<cit>(bf); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> cr ## suf ## begin param const { return std::make_reverse_iterator<cit>(ef); } \
  macro_ternary(ce, constexpr, inline) std::reverse_iterator<cit> cr ## suf ## end param const { return std::make_reverse_iterator<cit>(bf); }

#define expand_iterators(param, it, cit, bf, ef, suf, ce) \
  expand_mutable_iterators(param, it, bf, ef, suf, ce) \
  expand_const_iterators(param, cit, bf, ef, suf, ce)

#define expand_reverse_iterators(param, it, cit, bf, ef, suf, ce) \
  expand_mutable_reverse_iterators(param, it, bf, ef, suf, ce) \
  expand_const_reverse_iterators(param, cit, bf, ef, suf, ce)

#define expand_all_iterators(param, it, cit, bf, ef, suf, ce) \
  expand_iterators(param, it, cit, bf, ef, suf, ce) \
  expand_reverse_iterators(param, it, cit, bf, ef, suf, ce)

#define expand_all_mutable_iterators(param, it, bf, ef, suf, ce) \
  expand_mutable_iterators(param, it, bf, ef, suf, ce) \
  expand_mutable_reverse_iterators(param, it, bf, ef, suf, ce)

#define expand_all_const_iterators(param, cit, bf, ef, suf, ce) \
  expand_const_iterators(param, cit, bf, ef, suf, ce) \
  expand_const_reverse_iterators(param, cit, bf, ef, suf, ce)

// Time logging functions
#define time_start(tid, msg) \
  [[maybe_unused]] auto const __msg_ ## tid { msg }; \
  [[maybe_unused]] auto __time_ ## tid = std::chrono::high_resolution_clock::now(); \

#define time_msg(tid) __msg_ ## tid

#define time_end(tid) \
  std::chrono::duration<double>{ \
    std::chrono::high_resolution_clock::now() - (__time_ ## tid) \
  }.count()
