#pragma once

#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#include <typeinfo>

namespace util {

  // Calls an element destructor
  template <class T>
  void destruct (T& el) {
    el.~T();
  }

  #ifdef __GNUG__

  // Gets the name of T
  // Thanks to https://stackoverflow.com/a/4541470/6441345
  template <typename T>
  std::string type_name (void) {
    char const* name = typeid(T).name();
    int status;

    // Build a unique_ptr of the demangled name
    std::unique_ptr<char, void(*)(void*)> res{
      abi::__cxa_demangle(name, nullptr, nullptr, &status),
      std::free
    };

    // Returns a string
    return (status == 0) ? res.get() : name;
  }

  // Gets the name of T
  template <typename T>
  std::string type_name (T const&) { return type_name<T>(); }

  #endif

};
