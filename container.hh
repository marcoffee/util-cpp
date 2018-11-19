#pragma once

#include <vector>
#include <deque>
#include <array>
#include <list>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <string>
#include <string_view>
#include <iterator>
#include <iostream>
#include "util_constexpr.hh"

namespace util {

  struct default_streamer {
    template <typename T>
    void operator () (std::ostream& out, T const& value) const {
      out << value;
    }
  };

  // Class to print containers
  template <typename T, typename P = default_streamer>
  class printer {
   public:
    using iterator = typename T::iterator;
    using const_iterator = typename T::const_iterator;
    using streamer = P;

   private:
    T const& cont_;
    uintmax_t limit_;
    std::string_view sep_, align_, border_, open_, close_;
    streamer stream_;

    // Internal function to print an iterator range
    void print_iterator (
      std::ostream& out, const_iterator beg, const_iterator end
    ) const {
      for (auto it = beg; it != end; ++it) {
        if (it != beg) {
          out << this->sep_;
        }

        out << this->align_;
        this->stream_(out, *it);
      }
    }

   public:
    // Constructor
    constexpr printer (
      T const& cont, uintmax_t limit = 0,
      std::string_view sep = ", ", std::string_view align = "",
      std::string_view border = "", std::string_view open = "",
      std::string_view close = ""
    ) : cont_{ cont }, limit_{ limit },
        sep_{ sep }, align_{ align }, border_{ border },
        open_{ open }, close_{ close } {}

    // Main print function
    std::ostream& print (std::ostream& out) const {
      out << this->open_;

      // Avoid printing borders when container is empty
      if (!this->empty()) {
        out << this->border_;

        // Test if it can print the whole container
        if (this->limit_ != 0 and this->size() > this->limit_) {
          uintmax_t const pieces = this->limit_ >> 1;

          // Break the container in two and print
          auto stop_it = std::next(this->begin(), pieces);
          auto rest_it = std::next(this->begin(), this->size() - pieces);

          this->print_iterator(out, this->begin(), stop_it);
          out << this->sep_ << this->align_ << "..." << this->sep_;
          this->print_iterator(out, rest_it, this->end());

        } else {
          // Print the whole container
          this->print_iterator(out, this->begin(), this->end());
        }

        out << this->border_;
      }

      out << this->close_;
      return out;
    }

    // Container's getters
    const_iterator begin (void) const { return std::begin(this->cont_); }
    const_iterator end (void) const { return std::end(this->cont_); }
    uintmax_t size (void) const { return this->cont_.size(); }
    bool empty (void) const { return this->cont_.empty(); }

    // Simple setters
    constexpr printer& limit (uintmax_t limit) {
      this->limit_ = limit;
      return *this;
    }

    constexpr printer& sep (std::string_view sep) {
      this->sep_ = sep;
      return *this;
    }

    constexpr printer& align (std::string_view align) {
      this->align_ = align;
      return *this;
    }

    constexpr printer& border (std::string_view border) {
      this->border_ = border;
      return *this;
    }

    constexpr printer& open (std::string_view open) {
      this->open_ = open;
      return *this;
    }

    constexpr printer& close (std::string_view close) {
      this->close_ = close;
      return *this;
    }
  };

  // Class to create a container based from begin and end pointers
  template <
    typename IT,
    typename CIT = IT,
    typename = std::enable_if_t<is_iterator_v<IT> and is_iterator_v<CIT>>
  >
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
    // Constructor based on pointers
    constexpr container (IT begin, IT end)
    : begin_{ begin }, end_{ end } {};

    // Constructor based on begin pointer and size
    constexpr container (IT begin, uintmax_t size)
    : container{ begin, std::next(begin, size) } {};

    // Container's getters
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
}

////////////////////////////////////////////////////////////////////////////////

// printer's printer :)

template <typename T>
std::ostream& operator << (std::ostream& out, util::printer<T> const& pc) {
  return pc.print(out);
}

// Helper functions for known containers

template <typename IT>
std::ostream& operator << (std::ostream& out, util::container<IT> const& con) {
  return out << util::printer(con, 20, ", ", "", " ", "[", "]");
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::vector<T> const& vec) {
  return out << util::printer(vec, 20, ", ", "", " ", "[", "]");
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::deque<T> const& vec) {
  return out << util::printer(vec, 20, ", ", "", " ", "[", "]");
}

template <typename T, uintmax_t N>
std::ostream& operator << (std::ostream& out, std::array<T, N> const& arr) {
  return out << util::printer(arr, 20, ", ", "", " ", "[", "]");
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::list<T> const& lst) {
  return out << util::printer(lst, 20, ", ", "", " ", "[", "]");
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::set<T> const& set) {
  return out << util::printer(set, 20, ", ", "", " ", "{", "}");
}

template <typename T>
std::ostream& operator << (std::ostream& out, std::unordered_set<T> const& set) {
  return out << util::printer(set, 20, ", ", "", " ", "{", "}");
}

// Required to print maps
template <typename T, typename U>
std::ostream& operator << (std::ostream& out, std::pair<T, U> const& p) {
  return out << "( " << p.first << " , " << p.second << " )";
}

template <typename K, typename V>
std::ostream& operator << (std::ostream& out, std::unordered_map<K, V> const& map) {
  return out << util::printer(map, 20, "\n", "\t", "\n", "{", "}");
}

template <typename K, typename V>
std::ostream& operator << (std::ostream& out, std::map<K, V> const& map) {
  return out << util::printer(map, 20, "\n", "\t", "\n", "{", "}");
}
