#pragma once

#include <limits>
#include <functional>
#include <type_traits>
#include "util_macro.hh"

namespace util {
  constexpr double PI = 3.141592653589793238463;
};

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
static T defref;

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

////////////////////////////////////////////////////////////////////////////////

template <typename R>
struct as_call { using type = R(); };

template <typename R, typename... ARGS>
struct as_call<R(ARGS...)> { using type = R(ARGS...); };

template <typename R>
using as_call_t = typename as_call<R>::type;

////////////////////////////////////////////////////////////////////////////////

template <uintmax_t, uintmax_t, typename, typename...>
struct _change_arg {};

template <uintmax_t P, uintmax_t A, typename C, typename T, typename... NXT>
struct _change_arg<P, A, C, T, NXT...> {

  template <typename... DON>
  struct _sub_change_arg : _change_arg<
    P, A + 1, C, NXT...
  >::template _sub_change_arg<DON..., std::conditional_t<P == A, C, T>> {};

};

template <uintmax_t A, uintmax_t P, typename C>
struct _change_arg<P, A, C> {

  template <typename... DON>
  struct _sub_change_arg {
    using type = typename std::tuple<DON...>;
  };

};

template <uintmax_t P, typename C, typename... ARGS>
struct change_arg : _change_arg<P, 0, C, ARGS...>::template _sub_change_arg<> {};

template <uintmax_t P, typename C, typename... ARGS>
using change_arg_t = typename change_arg<P, C, ARGS...>::type;


////////////////////////////////////////////////////////////////////////////////

template <typename, typename>
struct build_function {};

template <typename R, typename... ARGS>
struct build_function<R, std::tuple<ARGS...>> {
  using type = std::function<R(ARGS...)>;
};

template <typename R, typename... ARGS>
using build_function_t = typename build_function<R, ARGS...>::type;

////////////////////////////////////////////////////////////////////////////////

template <typename>
struct _function_traits {
  static constexpr bool is_function = false;
};

template <typename R, typename... ARGS>
struct _function_traits<std::function<R(ARGS...)>> {

  static constexpr bool is_function = true;
  using header = R(ARGS...);
  using ret_type = R;
  using as_tuple = std::tuple<ARGS...>;

  static const size_t count_args = sizeof...(ARGS);

  template <size_t N>
  using arg_type = typename std::tuple_element<N, as_tuple>::type;

  template <typename NR>
  struct ret_rebind {
    using type = std::function<NR(ARGS...)>;
  };

  template <size_t N, typename NA>
  using arg_rebind = build_function_t<R, change_arg_t<N, NA, ARGS...>>;

};

template <typename F>
struct function_traits : _function_traits<std::decay_t<F>> {};

template <typename F, size_t N, typename NR>
using function_arg_rebind = typename function_traits<F>::template arg_rebind<N, NR>;

template <typename F, typename NR>
using function_ret_rebind = typename function_traits<F>::template ret_rebind<NR>;

////////////////////////////////////////////////////////////////////////////////

template <typename>
struct _noop {};

template <>
struct _noop<void()> {
  static constexpr void value (void) {}
};

template <typename R, typename... ARGS>
struct _noop<std::function<R(ARGS...)>> {
  static R const& value (ARGS&&...) { return defref<R>; }
};

template <typename... ARGS>
struct _noop<std::function<void(ARGS...)>> {
  static void value (ARGS&&...) {}
};

template <typename R, typename... ARGS>
struct _noop<R(ARGS...)> {
  static R const& value (ARGS&&...) { return defref<R>; };
};

template <typename... ARGS>
struct _noop<void(ARGS...)> { static constexpr void value (ARGS&&...) {}; };

template <typename T = void()>
static const std::function<T> noop = _noop<T>::value;

template <typename R, typename... ARGS>
static const std::function<R(ARGS...)>
  noop<std::function<R(ARGS...)>> = _noop<std::function<R(ARGS...)>>::value;
