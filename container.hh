#pragma once

#include <list>
#include <unordered_map>
#include <string>
#include <string_view>
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
  std::string_view sep = ", ", std::string_view align = ""
) {
  for (IT it = begin; it != end; ++it) {
    if (it != begin) {
      out << sep;
    }

    out << align << *it;
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class print_container {
  T const& cont;
  uintmax_t limit_;
  std::string_view sep_, align_, border_, open_, close_;

  template <typename U>
  friend std::ostream& operator << (std::ostream&, print_container<U> const&);

 public:
  constexpr print_container (
    T const& cont, uintmax_t limit = 0,
    std::string_view sep = ", ", std::string_view align = "",
    std::string_view border = "", std::string_view open = "",
    std::string_view close = ""
  ) : cont{ cont }, limit_{ limit },
      sep_{ sep }, align_{ align }, border_{ border },
      open_{ open }, close_{ close } {}

  constexpr print_container& sep (std::string_view sep) {
    this->sep_ = sep;
    return *this;
  }

  constexpr print_container& align (std::string_view align) {
    this->align_ = align;
    return *this;
  }

  constexpr print_container& border (std::string_view border) {
    this->border_ = border;
    return *this;
  }

  constexpr print_container& open (std::string_view open) {
    this->open_ = open;
    return *this;
  }

  constexpr print_container& close (std::string_view close) {
    this->close_ = close;
    return *this;
  }

  constexpr print_container& limit (uintmax_t limit) {
    this->limit_ = limit;
    return *this;
  }
};

template <typename T>
std::ostream& operator << (std::ostream& out, print_container<T> const& pc) {
  out << pc.open_;

  if (!pc.cont.empty()) {
    out << pc.border_;

    if (pc.limit_ != 0 and pc.cont.size() > pc.limit_) {
      uintmax_t const pieces = pc.limit_ >> 1;
      auto stop_it = pc.cont.begin(), rest_it = pc.cont.begin();

      std::advance(stop_it, pieces);
      std::advance(rest_it, pc.cont.size() - pieces);

      print_iterator(out, pc.cont.begin(), stop_it, pc.sep_, pc.align_);
      out << pc.sep_ << pc.align_ << "..." << pc.sep_;
      print_iterator(out, rest_it, pc.cont.end(), pc.sep_, pc.align_);
    } else {
      print_iterator(out, pc.cont.begin(), pc.cont.end(), pc.sep_, pc.align_);
    }

    out << pc.border_;
  }

  out << pc.close_;
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

  using value_type = typename std::iterator_traits<iterator>::value_type;
  using iterator_category = typename std::iterator_traits<iterator>::iterator_category;

private:
  IT begin_, end_;

public:
  constexpr container (IT begin, IT end) : begin_{ begin }, end_{ end } {};

  constexpr bool empty (void) const { return this->begin_ == this->end_; }

  constexpr uintmax_t size (void) const {
    return std::distance(this->begin_, this->end_);
  }

  std::enable_if_t<
    std::is_same_v<iterator_category, std::random_access_iterator_tag>,
    value_type const&
  > operator [] (intmax_t idx) { return *(this->begin_ + idx); }

  expand_all_iterators((void), iterator, const_iterator, this->begin_, this->end_, EMPTY_ARG, true);
};

template <
  typename IT, typename CIT = IT,
  typename = std::enable_if_t<is_iterator_v<IT> and is_iterator_v<CIT>>
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
