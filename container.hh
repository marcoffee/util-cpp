#pragma once

#include <list>
#include <unordered_map>
#include <string>
#include <iterator>
#include <iostream>
#include "util_constexpr.hh"

extern std::string
  default_sep, default_align, default_border, default_open, default_close,
  vector_sep, vector_align, vector_border, vector_open, vector_close,
  map_sep, map_align, map_border, map_open, map_close;

extern uintmax_t default_max_print, vector_max_print, map_max_print;

template <typename IT, typename = typename std::enable_if_t<
  !std::is_same_v<typename std::iterator_traits<IT>::value_type, void>
>>
void print_iterator (
  std::ostream& out, IT begin, IT end,
  std::string const& sep = ", ", std::string const& align = ""
) {
  for (IT it = begin; it != end; ++it) {
    if (it != begin) {
      out << sep;
    }

    out << align << *it;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename CONT>
struct print_container {
  CONT const& cont;
  uintmax_t max_size;
  std::string const& sep, align, border, open, close;

  print_container (
    CONT const& cont, uintmax_t max_size,
    std::string const& sep = ", ", std::string const& align = "",
    std::string const& border = "", std::string const& open = "",
    std::string const& close = ""
  ) : cont(cont), max_size(max_size),
      sep(sep), align(align), border(border), open(open), close(close) {}
};

template <typename CONT>
std::ostream& operator << (std::ostream& out, print_container<CONT> const& pc) {
  out << pc.open;

  if (!pc.cont.empty()) {
    out << pc.border;

    if (pc.max_size != 0 && pc.cont.size() > pc.max_size) {
      uintmax_t const pieces = pc.max_size >> 1;
      auto stop_it = pc.cont.begin(), rest_it = pc.cont.begin();

      std::advance(stop_it, pieces);
      std::advance(rest_it, pc.cont.size() - pieces);

      print_iterator(out, pc.cont.begin(), stop_it, pc.sep, pc.align);
      out << pc.sep << pc.align << "..." << pc.sep;
      print_iterator(out, rest_it, pc.cont.end(), pc.sep, pc.align);
    } else {
      print_iterator(out, pc.cont.begin(), pc.cont.end(), pc.sep, pc.align);
    }

    out << pc.border;
  }

  out << pc.close;
  return out;
}

////////////////////////////////////////////////////////////////////////////////

template <typename IT, typename CIT = IT>
class container {

public:
  using iterator = IT;
  using const_iterator = CIT;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  IT _begin, _end;

public:
  constexpr container (IT begin, IT end) : _begin(begin), _end(end) {};

  constexpr bool empty (void) const { return this->_begin == this->_end; }

  constexpr uintmax_t size (void) const {
    return std::distance(this->_begin, this->_end);
  }

  expand_all_iterators((void), iterator, const_iterator, this->_begin, this->_end, EMPTY_ARG, true);
};

template <
  typename IT, typename CIT = IT,
  typename = std::enable_if_t<is_iterator_v<IT> && is_iterator_v<CIT>>
>
constexpr container<IT, CIT> make_container (IT beg, IT end) {
  return container<IT, CIT>(beg, end);
}

template <
  typename IT, typename CIT = IT,
  typename = std::enable_if_t<std::is_pointer_v<IT>, container<IT, CIT>>
>
constexpr container<IT, CIT> make_container (IT ptr, uintmax_t size) {
  return container<IT, CIT>(ptr, ptr + size);
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
std::ostream& operator << (std::ostream& out, std::pair<T, U> const& p) {
  return out << "( " << p.first << " , " << p.second << " )";
}

template <typename IT>
std::ostream& operator << (std::ostream& out, container<IT> const& cont) {
  return out << print_container(
    cont, default_max_print, default_sep, default_align,
    default_border, default_open, default_close
  );
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::vector<T> const& vec) {
  return out << print_container(
    vec, vector_max_print, vector_sep, vector_align,
    vector_border, vector_open, vector_close
  );
}

template <typename T, uintmax_t N>
std::ostream& operator << (std::ostream& out, std::array<T, N> const& arr) {
  return out << print_container(
    arr, vector_max_print, vector_sep, vector_align,
    vector_border, vector_open, vector_close
  );
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::list<T> const& lst) {
  return out << print_container(
    lst, vector_max_print, vector_sep, vector_align,
    vector_border, vector_open, vector_close
  );
}

template <typename K, typename V>
std::ostream& operator << (std::ostream& out, std::unordered_map<K, V> const& map) {
  return out << print_container(
    map, map_max_print, map_sep, map_align,
    map_border, map_open, map_close
  );
}
