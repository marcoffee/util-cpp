#pragma once

namespace util {

  template <class T>
  void destruct (T& el) {
    el.~T();
  }

};
