#pragma once

#include <iomanip>
#include <vector>
#include <sstream>
#include <string>
#include <string_view>
#include "util_macro.hh"

extern std::string_view _ref_sv;

constexpr std::string_view lsplit (
  std::string_view str, std::string_view const& delim,
  std::string_view& rem = _ref_sv
) {
  uintmax_t const pos = str.find_first_of(delim);

  if (pos == std::string_view::npos) {
    rem = "";
    return str;
  }

  rem = str.substr(pos + 1);
  return str.substr(0, pos);
}

template <typename D = std::string_view>
constexpr std::string_view rsplit (
  std::string_view str, std::string_view const& delim,
  std::string_view& rem = _ref_sv
) {
  uintmax_t const pos = str.find_last_of(delim);

  if (pos == std::string_view::npos) {
    rem = "";
    return str;
  }

  rem = str.substr(0, pos);
  return str.substr(pos + 1);
}

constexpr auto split = lsplit;

std::vector<std::string_view> split_all (
  std::string_view str, std::string_view const& delim,
  bool left = true, uintmax_t limit = 0
);

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::string f_to_string (
  T const val, uintmax_t prec = std::numeric_limits<T>::max_digits10
) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(prec) << val;
    return out.str();
}

////////////////////////////////////////////////////////////////////////////////

constexpr std::string_view ltrim (
  std::string_view const& str, std::string_view const& del = WHITESPACE
) {
  uintmax_t const pos = std::min(str.find_first_not_of(del), str.size());
  return str.substr(pos);
}

constexpr std::string_view rtrim (
  std::string_view const& str, std::string_view const& del = WHITESPACE
) {
  uintmax_t const pos = std::min(str.find_last_not_of(del), str.size());
  return str.substr(0, std::min(pos + 1, str.size()));
}

constexpr std::string_view trim (
  std::string_view const& str, std::string_view const& del = WHITESPACE
) {
  return rtrim(ltrim(str, del), del);
}
