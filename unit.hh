#pragma once

#include "util_constexpr.hh"

namespace util::unit {
  // Convert a number to a decimal unit
  template <typename T>
  std::tuple<double, std::string_view, std::string_view> decimal (
    T const& value
  ) {
    constexpr double ten = double{ 10.0 },
      yocto = util::pow<-24>(ten), zepto = util::pow<-21>(ten),
      atto  = util::pow<-18>(ten), femto = util::pow<-15>(ten),
      pico  = util::pow<-12>(ten), nano  = util::pow< -9>(ten),
      micro = util::pow< -6>(ten), milli = util::pow< -3>(ten),
      kilo  = util::pow<  3>(ten), mega  = util::pow<  6>(ten),
      giga  = util::pow<  9>(ten), tera  = util::pow< 12>(ten),
      peta  = util::pow< 15>(ten), exa   = util::pow< 18>(ten),
      zeta  = util::pow< 21>(ten), yotta = util::pow< 24>(ten);

    if (value >= yotta) {
      return { value / yotta, "Y", "yotta" };

    } else if (value >= zeta) {
      return { value / zeta, "Z", "zeta" };

    } else if (value >= exa) {
      return { value / exa, "E", "exa" };

    } else if (value >= peta) {
      return { value / peta, "P", "peta" };

    } else if (value >= tera) {
      return { value / tera, "T", "tera" };

    } else if (value >= giga) {
      return { value / giga, "G", "giga" };

    } else if (value >= mega) {
      return { value / mega, "M", "mega" };

    } else if (value >= kilo) {
      return { value / kilo, "k", "kilo" };

    } else if (value >= 1) {
      return { value, "", "" };

    } else if (value >= milli) {
      return { value / milli, "m", "milli" };

    } else if (value >= micro) {
      return { value / micro, "u", "micro" };

    } else if (value >= nano) {
      return { value / nano, "n", "nano" };

    } else if (value >= pico) {
      return { value / pico, "p", "pico" };

    } else if (value >= femto) {
      return { value / femto, "f", "femto" };

    } else if (value >= atto) {
      return { value / atto, "a", "atto" };

    } else if (value >= zepto) {
      return { value / zepto, "z", "zepto" };
    }

    return { value / yocto, "y", "yocto" };
  }

  // Convert a number to a binary unit
  template <typename T>
  std::tuple<double, std::string_view, std::string_view> binary (
    T const& value
  ) {
    constexpr double two = double{ 2.0 },
      kibi = util::pow<10>(two), mibi = util::pow<20>(two),
      gibi = util::pow<30>(two), tebi = util::pow<40>(two),
      pebi = util::pow<50>(two), exbi = util::pow<60>(two),
      zebi = util::pow<70>(two), yobi = util::pow<80>(two);

    if (value >= yobi) {
      return { value / yobi, "Yi", "yobi" };

    } else if (value >= zebi) {
      return { value / zebi, "Zi", "zebi" };

    } else if (value >= exbi) {
      return { value / exbi, "Ei", "exbi" };

    } else if (value >= pebi) {
      return { value / pebi, "Pi", "pebi" };

    } else if (value >= tebi) {
      return { value / tebi, "Ti", "tebi" };

    } else if (value >= gibi) {
      return { value / gibi, "Gi", "gibi" };

    } else if (value >= mibi) {
      return { value / mibi, "Mi", "mibi" };

    } else if (value >= kibi) {
      return { value / kibi, "Ki", "kibi" };
    }

    return { value, "", "" };
  }
};
