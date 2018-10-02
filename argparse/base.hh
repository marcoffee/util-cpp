#pragma once

#include <cctype>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "exception.hh"

namespace util {

  template <typename T>
  using string_map = std::unordered_map<std::string, T>;
  template <typename T>
  using string_pair = std::pair<std::string, T>;

  using string_set = std::unordered_set<std::string>;
  using string_mapset = string_map<string_set>;
  using string_vector = std::vector<std::string>;

  class argparse {

   public:
    class params;

    class option {
      uint8_t value_;

     public:
      static constexpr uint8_t
        none = 0b00000U, required = 0b00001U, multiple = 0b00010U,
        enabler = 0b00100U, disabler = 0b01000U, counter = 0b10000U;

      constexpr option (uint8_t value = none) : value_{ value } {};

      constexpr bool is_required (void) const { return this->value_ & required; }
      constexpr bool is_multiple (void) const { return this->value_ & multiple; }
      constexpr bool is_enabler (void) const { return this->value_ & enabler; }
      constexpr bool is_disabler (void) const { return this->value_ & disabler; }
      constexpr bool is_counter (void) const { return this->value_ & counter; }

      constexpr option operator | (option const& ot) const { return this->value_ | ot.value_; }
      constexpr option operator | (uint8_t const& ot) const { return this->value_ | ot; }

      constexpr option& operator |= (option const& ot) { this->value_ |= ot.value_; return *this; }
      constexpr option& operator |= (uint8_t const& ot) { this->value_ |= ot; return *this; }
    };

    static bool valid_name (std::string const& name);

   private:
    string_map<option> options_;
    string_mapset choices_;
    string_map<std::string> default_;
    string_set non_default_;
    string_vector positional_;

    void assert_param_exists (std::string const& name) const;
    void assert_param_does_not_exists (std::string const& name) const;
    void assert_param_name_valid (std::string const& name) const;
    void assert_param_positional (std::string const& name, option const& opt) const;
    void assert_options (std::string const& name, option const& opt) const;
    void assert_non_default (std::string const& name, option const& opt) const;
    void assert_choices (std::string const& name, option const& opt, string_set const& choices) const;
    void assert_choices_arg (std::string const& name, std::string const& param) const;
    void assert_param_required (std::string const& name, option const& opt, params const& args) const;

    void fix_assert_new_param (std::string const& name, option& opt, string_set const& choices);

   public:
    argparse (void) {}

    params parse (uintmax_t const& argc, char const* const* argv);

    void add (std::string const& name, std::string def, option opt = option::none, string_set const& choices = {});
    void add (std::string const& name, option opt = option::none, string_set const& choices = {});

    void add (std::string const& name, std::string def, string_set const& choices) {
      this->add(name, def, option::none, choices);
    }

    void add (std::string const& name, string_set const& choices) {
      this->add(name, option::none, choices);
    }

    inline bool exists (std::string const& name) const {
      return this->options_.count(name);
    }

    inline bool maybe_arg (std::string const& name) const {
      return name[0] == '-' and this->exists(name);
    }

  };

  class argparse::params {

   public:
    template <typename T>
    using conversor = std::function<T(std::string const&)>;

   private:
    friend class argparse;
    static string_vector const empty_vector;

    string_map<string_vector> data_;
    string_map<bool> set_;
    using map_iterator = string_vector::const_iterator;

    template <typename T>
    class iterator
    : public std::iterator<std::bidirectional_iterator_tag, T const> {

      map_iterator real_;
      conversor<T> convert_;

     public:
      iterator (void) {}

      iterator (map_iterator const real, conversor<T> const& convert)
      : real_{ real }, convert_{ convert } {}

      inline iterator& operator ++ (void) { ++this->real_; return *this; }
      inline iterator& operator -- (void) { --this->real_; return *this; }

      inline iterator operator ++ (int) { return iterator(this->real_++, this->convert_); }
      inline iterator operator -- (int) { return iterator(this->real_--, this->convert_); }

      inline bool operator == (iterator const& ot) { return this->real_ == ot.real_; }
      inline bool operator != (iterator const& ot) { return this->real_ != ot.real_; }

      inline T const operator * (void) { return this->convert_(*this->real_); }
      inline T const operator -> (void) { return **this; }

    };

   public:
    template <typename T>
    static T default_conversor (std::string const& value) {
      if constexpr (std::is_same_v<T, std::string>) {
        return value;
      } else if constexpr (std::is_floating_point_v<T>) {
        return std::stold(value);
      } else if constexpr (std::is_same_v<T, bool>) {
        return !(value.empty() or value == "0");
      } else if constexpr (std::is_unsigned_v<T>) {
        return std::stoull(value);
      } else if constexpr (std::is_signed_v<T>) {
        return std::stoll(value);
      } else if constexpr (std::is_constructible_v<T, std::string>) {
        return T(value);
      }

      throw std::domain_error("Unsupported type.");
    }

    params (
      string_map<std::string> const& def = {},
      string_set const& non_default = {}
    );

    bool is_set (std::string const& name) const;

    void set (std::string const& name, std::string const& val, bool replace = false);

    std::string& get_ref (std::string const& name);
    std::string const& get_ref (std::string const& name) const;

    template <typename T = std::string const&>
    T get (std::string const& name, conversor<T> const& convert = params::default_conversor<T>) const;

    template <typename T = std::string const&>
    T get (std::string const& name, T const& def, conversor<T> const& convert = params::default_conversor<T>) const;

    template <typename T = std::string>
    T const& get (std::string const& name, string_map<T> const& convert, T const& def) const;

    template <typename T = std::string>
    bool get (std::string const& name, iterator<T>& beg, iterator<T>& end, conversor<T> const& convert = params::default_conversor<T>) const;

    friend std::ostream& operator << (std::ostream& out, params const& args);

  };

  template <typename T>
  T argparse::params::get (std::string const& name, conversor<T> const& convert) const {
    return convert(this->get_ref(name));
  }

  template <typename T>
  T argparse::params::get (std::string const& name, T const& def, conversor<T> const& convert) const {
    if (this->is_set(name)) {
      return this->get<T>(name, convert);
    }

    return def;
  }

  template <typename T>
  T const& argparse::params::get (std::string const& name, string_map<T> const& convert, T const& def) const {
    std::string const& value = this->get_ref(name);
    auto it = convert.find(value);

    if (it != convert.end()) {
      return it->second;
    }

    return def;
  }

  template <typename T>
  bool argparse::params::get (std::string const& name, iterator<T>& beg, iterator<T>& end, conversor<T> const& convert) const {
    auto it = this->data_.find(name);

    if (it != this->data_.end()) {
      beg = iterator<T>(it->second.begin(), convert);
      end = iterator<T>(it->second.end(), convert);
      return true;
    }

    beg = iterator<T>(empty_vector.begin(), convert);
    beg = iterator<T>(empty_vector.end(), convert);

    return false;
  }

}
