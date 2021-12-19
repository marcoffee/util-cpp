#pragma once

#include <limits>
#include <iomanip>
#include <vector>
#include <sstream>
#include <string>
#include <string_view>
#include "util_macro.hh"

namespace util::string {

  extern std::string_view ref_sv;
  constexpr std::string_view whitespace{ WHITESPACE };

  // Defines the split function types
  using split_func = std::string_view (*) (
    std::string_view, std::string_view const&, std::string_view&
  );

  // Splits a string on the leftmost occurence of a delimiter
  constexpr std::string_view lsplit (
    std::string_view str, std::string_view const& delim,
    std::string_view& rem = ref_sv
  ) {
    // Find delimiter
    uintmax_t const pos = str.find_first_of(delim);

    if (pos == std::string_view::npos) {
      // Delimiter does not exist
      rem = "";
      return str;
    }

    rem = str.substr(pos + 1);
    return str.substr(0, pos);
  }

  // Splits a string on the rightmost occurence of a delimiter
  constexpr std::string_view rsplit (
    std::string_view str, std::string_view const& delim,
    std::string_view& rem = ref_sv
  ) {
    // Find delimiter
    uintmax_t const pos = str.find_last_of(delim);

    if (pos == std::string_view::npos) {
      // Delimiter does not exist
      rem = "";
      return str;
    }

    rem = str.substr(0, pos);
    return str.substr(pos + 1);
  }

  template <typename... ARGS>
  constexpr std::string_view split (ARGS&&... args) {
    return lsplit(std::forward<ARGS>(args)...);
  }

////////////////////////////////////////////////////////////////////////////////

  // Splits a string in all occurences of a delimiter, given the split function
  template <typename IT>
  constexpr void func_split_all (
    split_func func, std::string_view str, std::string_view const& delim, IT to,
    uintmax_t limit = std::string_view::npos
  ) {
    // Splits the string until it is empty or it reaches the limit
    for (uintmax_t count = 0; count < limit and !str.empty(); ++count) {
      // Stores the tokens on the iterator
      *(to++) = func(str, delim, str);
    }
  }

  // Splits a string in all occurences of a delimiter from left to right
  template <typename IT>
  constexpr void lsplit_all (
    std::string_view str, std::string_view const& delim, IT to,
    uintmax_t limit = std::string_view::npos
  ) {
    return func_split_all(lsplit, str, delim, to, limit);
  }

  // Splits a string in all occurences of a delimiter from right to left
  template <typename IT>
  constexpr void rsplit_all (
    std::string_view str, std::string_view const& delim, IT to,
    uintmax_t limit = std::string_view::npos
  ) {
    return func_split_all(rsplit, str, delim, to, limit);
  }

  // Splits a string in all occurences of a delimiter from left to right
  template <typename IT>
  constexpr void split_all (
    std::string_view str, std::string_view const& delim, IT to,
    uintmax_t limit = std::string_view::npos
  ) {
    return lsplit_all(str, delim, to, limit);
  }

  // Splits a string in all occurences of a delimiter from left to right
  // and stores the results on a vector
  inline std::vector<std::string_view> lsplit_all (
    std::string_view str, std::string_view const& delim,
    uintmax_t limit = std::string_view::npos
  ) {
    std::vector<std::string_view> result;
    lsplit_all(str, delim, std::back_inserter(result), limit);
    return result;
  }

  // Splits a string in all occurences of a delimiter from right to left
  // and stores the results on a vector
  inline std::vector<std::string_view> rsplit_all (
    std::string_view str, std::string_view const& delim,
    uintmax_t limit = std::string_view::npos
  ) {
    std::vector<std::string_view> result;
    rsplit_all(str, delim, std::back_inserter(result), limit);
    return result;
  }

  // Splits a string in all occurences of a delimiter from left to right
  // and stores the results on a vector
  inline std::vector<std::string_view> split_all (
    std::string_view str, std::string_view const& delim,
    uintmax_t limit = std::string_view::npos
  ) {
    return lsplit_all(str, delim, limit);
  }

////////////////////////////////////////////////////////////////////////////////

  // Removes selected characters from left of string
  constexpr std::string_view ltrim (
    std::string_view const& str, std::string_view const& del = whitespace
  ) {
    uintmax_t const pos = std::min(str.find_first_not_of(del), str.size());
    return str.substr(pos);
  }

  // Removes selected characters from right of string
  constexpr std::string_view rtrim (
    std::string_view const& str, std::string_view const& del = whitespace
  ) {
    uintmax_t const pos = std::min(str.find_last_not_of(del), str.size());
    return str.substr(0, std::min(pos + 1, str.size()));
  }

  // Removes selected characters from left and right of string
  constexpr std::string_view trim (
    std::string_view const& str, std::string_view const& del = whitespace
  ) {
    return rtrim(ltrim(str, del), del);
  }

////////////////////////////////////////////////////////////////////////////////

  // Converts from any output streamable type to string
  template <typename T, typename... ARGS>
  std::string from (T const& val, ARGS&&... args) {
    std::ostringstream out;

    // Fold expression
    (out << ... << args) << val;
    return out.str();
  }

  // Converts from floating point type to string
  template <
    typename T,
    typename = typename std::enable_if_t<std::is_floating_point_v<T>>,
    typename... ARGS
  >
  std::string fromf (
    T const& val, uintmax_t prec = std::numeric_limits<T>::max_digits10 + 1,
    ARGS&&... args
  ) {
    std::ostringstream out;

    // Fold expression
    (out << ... << args) << std::fixed << std::setprecision(prec) << val;
    return out.str();
  }

  // Converts from integer to string
  template <
    typename T,
    typename = typename std::enable_if_t<std::is_integral_v<T>>,
    typename... ARGS
  >
  std::string fromi (
    T const val, uintmax_t width = 0, char fill = ' ', ARGS&&... args
  ) {
    std::ostringstream out;

    // Fold expression
    (out << ... << args) << std::setfill(fill) << std::setw(width) << val;
    return out.str();
  }

  // Converts from string to any default-constructible, input streamable type
  template <typename T>
  T to (std::string const& str) {
    std::istringstream inp{ str };
    T result;

    inp >> result;
    return result;
  }

  // Concatenates multiple objects into one string
  template <typename... ARGS>
  std::string concat (ARGS&&... args) {
    std::ostringstream out;

    (out << ... << args);
    return out.str();
  }
};
