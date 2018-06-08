#pragma once

#include <limits>
#include <functional>
#include <type_traits>
#include "util_macro.hh"

template <uintmax_t N>
struct static_log2 {
  constexpr static uintmax_t value = static_log2<(N >> 1)>::value + 1;
};

template <>
struct static_log2<1> {
  constexpr static uintmax_t value = 0;
};

template <uintmax_t N>
constexpr uintmax_t static_log2_v = static_log2<N>::value;

////////////////////////////////////////////////////////////////////////////////

template <
  typename T,
  typename = typename std::enable_if_t<std::is_integral<T>::value>
>
struct count_bits {
  static constexpr uintmax_t
    value = std::numeric_limits<std::make_unsigned_t<T>>::digits;
};

template <typename T>
constexpr uintmax_t count_bits_v = count_bits<T>::value;

////////////////////////////////////////////////////////////////////////////////

template <bool, typename A, typename B>
struct type_ternary { using type = A; };

template <typename A, typename B>
struct type_ternary<false, A, B> { using type = B; };

template <bool c, typename A, typename B>
using type_ternary_t = typename type_ternary<c, A, B>::type;

////////////////////////////////////////////////////////////////////////////////

template <typename T, T patt, uintmax_t size, uintmax_t pos = 0, uintmax_t avai = count_bits_v<T>>
struct bitwise_repeat {
  static constexpr T value = bitwise_repeat<T, patt, size, pos + size, (avai > size) ? (avai - size) : 0>::value | (patt << pos);
};

template <typename T, T patt, uintmax_t size, uintmax_t pos>
struct bitwise_repeat<T, patt, size, pos, 0> {
  static constexpr T value = 0;
};

template <typename T, T patt, uintmax_t size>
constexpr T bitwise_repeat_v = bitwise_repeat<T, patt, size>::value;

////////////////////////////////////////////////////////////////////////////////

template <typename T, uintmax_t pos, bool less>
struct make_pattern {
private:
  static constexpr uintmax_t one = 1;
  static constexpr uintmax_t bits = one << pos;
  static constexpr uintmax_t block = (one << bits) - one;
  static constexpr uintmax_t patt = less ? block : (block << bits);

public:
  static constexpr T value = bitwise_repeat_v<T, patt, bits << one>;
};

template <typename T, bool less>
struct make_pattern<T, 1, less> {
private:
  static constexpr T patt = less ? 0x3 : 0xC;

public:
  static constexpr T value = bitwise_repeat_v<T, patt, 4>;
};

template <typename T, bool less>
struct make_pattern<T, 0, less> {
private:
  static constexpr T patt = less ? 0x5 : 0xA;

public:
  static constexpr T value = bitwise_repeat_v<T, patt, 4>;
};

template <typename T, uintmax_t pos, bool less = false>
constexpr T make_pattern_v = make_pattern<T, pos, less>::value;

////////////////////////////////////////////////////////////////////////////////

template <typename T, uintmax_t pos, bool less, T... vals>
struct make_pattern_array : make_pattern_array<
  T, pos - 1, less, vals..., make_pattern_v<T, pos - 1, less>
> {};

template <typename T, bool less, T... vals>
struct make_pattern_array<T, 0, less, vals...> {
private:
  static constexpr uintmax_t l_shift = static_log2_v<count_bits_v<T>>;

public:
  static constexpr std::array<T, l_shift> value = { vals... };
};

template <typename T, bool less = false>
struct pattern_array : make_pattern_array<
  T, static_log2_v<count_bits_v<T>>, less
> {};

template <typename T, bool less = false>
constexpr std::array<T, static_log2_v<count_bits_v<T>>>
  pattern_array_v = pattern_array<T, less>::value;

////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct _defref {
public:
  static T value;
};

template <typename T>
T _defref<T>::value;

template <typename T>
T& _defref_v = _defref<T>::value;

////////////////////////////////////////////////////////////////////////////////

template <
  typename T, uintmax_t i = 0,
  typename = typename std::enable_if_t<std::is_integral_v<T>>
>
constexpr T popcount (T n) {
  if constexpr (i < static_log2_v<count_bits_v<T>>) {
    return popcount<T, i + 1>(
      (n & make_pattern_v<T, i, true>) + (
      (n & make_pattern_v<T, i, false>) >> (T(1) << i))
    );

  } else {
    return n;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct is_iterator {
  static constexpr bool
    value = !std::is_same_v<typename std::iterator_traits<T>::value_type, void>;
};

template <typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;
