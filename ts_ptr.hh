#pragma once

#include <mutex>
#include <atomic>
#include <algorithm>
#include <type_traits>
#include "util_constexpr.hh"

namespace memory_stats {
  void add_memory (uintmax_t count);
  void del_memory (uintmax_t count);

  uintmax_t now (void);
  uintmax_t peak (void);
  uintmax_t total (void);
};

template <typename T, bool enable_cast = false, bool mem_stats = true>
class ts_ptr {
public:
  constexpr static bool is_array = std::is_array<T>::value;
  using type = typename std::remove_extent<T>::type;
  using type_ptr = type*;

private:
  uintmax_t _size = 0;
  type_ptr _ptr = nullptr;
  uintmax_t *_count = nullptr;
  std::mutex *_mutex = nullptr;

  constexpr void init_storage (void) {
    this->_count = new uintmax_t(1);
    this->_mutex = new std::mutex();
  }

  constexpr void free (void) {
    if constexpr (ts_ptr::is_array) {
      delete[] this->_ptr;

      if constexpr (mem_stats) {
        memory_stats::del_memory(sizeof(type) * this->size());
      }

    } else {
      delete this->_ptr;

      if constexpr (mem_stats) {
        memory_stats::del_memory(sizeof(type));
      }
    }

    delete this->_count;

    this->_mutex->unlock();
    delete this->_mutex;
  }

  constexpr void dec (void) {
    if (this->_ptr != nullptr) {
      this->_mutex->lock();
      if (--(*this->_count) == 0) {
        this->free();
      } else {
        this->_mutex->unlock();
      }

      this->_size = 0;
      this->_ptr = nullptr;
      this->_count = nullptr;
      this->_mutex = nullptr;
    }
  }

  constexpr void inc (void) {
    if (this->_ptr != nullptr) {
      this->_mutex->lock();
      ++(*this->_count);
      this->_mutex->unlock();
    }
  }

public:

  constexpr void copy_meta (ts_ptr const& ot) {
    this->_ptr = ot._ptr;
    this->_count = ot._count;
    this->_mutex = ot._mutex;
    this->_size = ot._size;
  }

  constexpr ts_ptr& copy_from (ts_ptr const& ot) {
    this->dec();
    this->copy_meta(ot);
    this->inc();
    return *this;
  }

  constexpr ts_ptr& move_from (ts_ptr& ot) {
    this->dec();
    this->copy_meta(ot);
    this->inc();
    ot.dec();
    return *this;
  }

  constexpr ts_ptr (void) {}

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr static ts_ptr make (uintmax_t size, bool zero = false) {
    ts_ptr result;
    result.allocate(size, zero);
    return result;
  }

  template <typename X = void, typename = std::enable_if_t<!is_array, X>, typename... ARGS>
  constexpr static ts_ptr make (ARGS&&... args) {
    ts_ptr result;
    result.allocate(std::forward<ARGS>(args)...);
    return result;
  }

  constexpr ts_ptr (ts_ptr const& ot) { this->copy_from(ot); }
  constexpr ts_ptr (ts_ptr&& ot) { this->move_from(ot); }

  constexpr ts_ptr& operator = (ts_ptr const& ot) { return this->copy_from(ot); }
  constexpr ts_ptr& operator = (ts_ptr&& ot) { return this->move_from(ot); }

  ~ts_ptr (void) { this->dec(); }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr void allocate (uintmax_t size, bool zero = false) {
    this->dec();
    this->_size = size;

    if (zero) {
      this->_ptr = new type[size]();
    } else {
      this->_ptr = new type[size];
    }

    if constexpr (mem_stats) {
      memory_stats::add_memory(sizeof(type) * size);
    }

    this->init_storage();
  }

  template <typename X = void, typename = std::enable_if_t<!is_array, X>, typename... ARGS>
  constexpr void allocate (ARGS&&... args) {
    this->dec();
    this->_ptr = new type(std::forward<ARGS>(args)...);

    if constexpr (mem_stats) {
      memory_stats::add_memory(sizeof(type));
    }

    this->init_storage();
  }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr begin (void) { return this->_ptr; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr end (void) { return this->_ptr + this->size(); }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const begin (void) const { return this->_ptr; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const end (void) const { return this->_ptr + this->size(); }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const cbegin (void) const { return this->_ptr; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const cend (void) const { return this->_ptr + this->size(); }

  constexpr type_ptr get (void) { return this->_ptr; }
  constexpr type_ptr const get (void) const { return this->_ptr; }

  constexpr void release (void) { this->dec(); }

  template <typename I, typename = std::enable_if_t<is_array, I>>
  constexpr type& operator [] (I pos) { return this->_ptr[pos]; }

  template <typename I, typename = std::enable_if_t<is_array, I>>
  constexpr type const& operator [] (I pos) const { return this->_ptr[pos]; }

  constexpr type const& operator * (void) const { return *this->_ptr; }
  constexpr type& operator * (void) { return *this->_ptr; }

  template <typename X = void, typename = std::enable_if_t<enable_cast, X>>
  constexpr operator type_ptr (void) { return this->_ptr; }

  template <typename X = void, typename = std::enable_if_t<enable_cast, X>>
  constexpr operator const type_ptr (void) const { return this->_ptr; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr uintmax_t const& size (void) const { return this->_size; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr bool empty (void) const { return this->_size == 0; }

  constexpr uintmax_t const& count (void) const { return *this->_count; }

  constexpr operator bool (void) const { return this->_ptr != nullptr; }
};
