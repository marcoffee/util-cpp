#pragma once

#include "evolution.hh"

namespace util::evolution {

  template <typename T, bool>
  struct _traits {};

  template <typename T>
  struct _traits<T, false> {
      static constexpr bool is_valid = false;

      static constexpr bool has_mutation = false;
      static constexpr bool has_crossover = false;
      static constexpr bool has_population = false;
      static constexpr bool has_multi_objective = false;
  };

  template <typename T>
  struct _traits<T, true> {
    static constexpr bool is_valid = true;

    static constexpr bool has_mutation = T::has_mutation;
    static constexpr bool has_crossover = T::has_crossover;
    static constexpr bool has_population = T::has_population;
    static constexpr bool has_multi_objective = T::has_multi_objective;
  };

  template <typename T>
  struct traits : _traits<T, std::is_base_of_v<base, T>> {};

};
