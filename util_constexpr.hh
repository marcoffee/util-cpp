#pragma once

#include <limits>
#include <functional>
#include <type_traits>
#include "util_macro.hh"

namespace util {
  constexpr double PI = 3.141592653589793238463;

  // Tests if a type is a specialization of a template
  template <typename T, template <typename...> typename TMPL>
  struct is_specialization_of : std::false_type {};

  template <template <typename...> typename TMPL, typename... ARGS>
  struct is_specialization_of<TMPL<ARGS...>, TMPL> : std::true_type {};

  template <typename T, template <typename...> typename TMPL>
  constexpr bool is_specialization_of_v = is_specialization_of<T, TMPL>::value;

////////////////////////////////////////////////////////////////////////////////

  // Create static array (similar to experimental::make_array)
  template <typename T, T... sigs>
  struct make_array_st {
    constexpr static std::array<T, sizeof...(sigs)> const value{ sigs... };
  };

  template <typename T, T... sigs>
  constexpr auto const& make_array{ make_array_st<T, sigs...>::value };
};

// log2 at compile time
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

// Count bits of an integer type
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

// Repeat a pattern of bits on a bitmask
template <
  typename T, T patt, uintmax_t size,
  uintmax_t pos = 0, uintmax_t avai = count_bits_v<T>
>
struct bitwise_repeat {
  static constexpr T value = bitwise_repeat<
    T, patt, size, pos + size, (avai > size) ? (avai - size) : 0
  >::value | (patt << pos);
};

template <typename T, T patt, uintmax_t size, uintmax_t pos>
struct bitwise_repeat<T, patt, size, pos, 0> {
  static constexpr T value = 0;
};

template <typename T, T patt, uintmax_t size>
constexpr T bitwise_repeat_v = bitwise_repeat<T, patt, size>::value;

////////////////////////////////////////////////////////////////////////////////

// Create bit patterns meant to be used in popcount
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

// Create an array of bit patterns meant to be used in popcount
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

// Counts the number of set bits on a bitmask
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

// Type for default reference parameters on functions
template <typename T>
static T defref;

////////////////////////////////////////////////////////////////////////////////

// Tests if a type is an iterator
template <typename T>
struct is_iterator {
  static constexpr bool
    value = !std::is_same_v<typename std::iterator_traits<T>::value_type, void>;
};

template <typename T>
constexpr bool is_iterator_v = is_iterator<T>::value;

////////////////////////////////////////////////////////////////////////////////

// Change an argument from the variadic template list
template <uintmax_t, uintmax_t, typename, typename...>
struct change_arg_st {};

template <uintmax_t P, uintmax_t A, typename C, typename T, typename... NXT>
struct change_arg_st<P, A, C, T, NXT...> {

  template <typename... DON>
  struct sub_change_arg_st : change_arg_st<
    P, A + 1, C, NXT...
  >::template sub_change_arg_st<DON..., std::conditional_t<P == A, C, T>> {};

};

template <uintmax_t A, uintmax_t P, typename C>
struct change_arg_st<P, A, C> {

  template <typename... DON>
  struct sub_change_arg_st {
    using type = typename std::tuple<DON...>;
  };

};

template <uintmax_t P, typename C, typename... ARGS>
struct change_arg : change_arg_st<P, 0, C, ARGS...>::template
  sub_change_arg_st<> {};

template <uintmax_t P, typename C, typename... ARGS>
using change_arg_t = typename change_arg<P, C, ARGS...>::type;

////////////////////////////////////////////////////////////////////////////////

// Build a std::function from the templates
template <typename, typename>
struct build_function {};

template <typename R, typename... ARGS>
struct build_function<R, std::tuple<ARGS...>> {
  using type = std::function<R(ARGS...)>;
};

template <typename R, typename... ARGS>
using build_function_t = typename build_function<R, ARGS...>::type;

////////////////////////////////////////////////////////////////////////////////

// Some traits for std::function
template <typename>
struct function_traits_st {
  static constexpr bool is_function = false;
};

template <typename R, typename... ARGS>
struct function_traits_st<std::function<R(ARGS...)>> {

  static constexpr bool is_function = true;

  // Header type
  using header = R(ARGS...);
  // Return type
  using ret_type = R;
  // Arguments as a tuple
  using as_tuple = std::tuple<ARGS...>;

  // Number of arguments
  static const size_t count_args = sizeof...(ARGS);

  // Type of an argument
  template <size_t N>
  using arg_type = typename std::tuple_element<N, as_tuple>::type;

  // Change return type
  template <typename NR>
  struct ret_rebind {
    using type = std::function<NR(ARGS...)>;
  };

  // Change argument type
  template <size_t N, typename NA>
  using arg_rebind = build_function_t<R, change_arg_t<N, NA, ARGS...>>;

};

template <typename F>
struct function_traits : function_traits_st<std::decay_t<F>> {};

template <typename F, size_t N, typename NR>
using function_arg_rebind = typename function_traits<F>::template
  arg_rebind<N, NR>;

template <typename F, typename NR>
using function_ret_rebind = typename function_traits<F>::template
  ret_rebind<NR>;

////////////////////////////////////////////////////////////////////////////////

// No operation function
template <typename>
struct noop_st {};

template <>
struct noop_st<void()> {
  static constexpr void value (void) {}
};

template <typename R, typename... ARGS>
struct noop_st<std::function<R(ARGS...)>> {
  static R const& value (ARGS&&...) { return defref<R>; }
};

template <typename... ARGS>
struct noop_st<std::function<void(ARGS...)>> {
  static void value (ARGS&&...) {}
};

template <typename R, typename... ARGS>
struct noop_st<R(ARGS...)> {
  static R const& value (ARGS&&...) { return defref<R>; };
};

template <typename... ARGS>
struct noop_st<void(ARGS...)> { static constexpr void value (ARGS&&...) {}; };

template <typename T = void()>
static const std::function<T> noop{ noop_st<T>::value };

template <typename R, typename... ARGS>
static const std::function<R(ARGS...)>
  noop<std::function<R(ARGS...)>> = noop_st<std::function<R(ARGS...)>>::value;
