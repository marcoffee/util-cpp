#include "string.hh"

std::string_view _ref_sv;

std::vector<std::string_view> split_all (
  std::string_view str, std::string_view const& delim,
  bool left, uintmax_t limit
) {
  std::string_view out;
  std::vector<std::string_view> result;
  uintmax_t count = 0;

  while (!str.empty()) {
    out = left ? lsplit(str, delim, str) : rsplit(str, delim, str);
    result.emplace_back(std::move(out));

    if (++count == limit) {
      break;
    }
  }

  return result;
}
