#pragma once

#include <mutex>
#include <atomic>
#include <algorithm>
#include <memory>
#include <type_traits>
#include "util_constexpr.hh"

template <
  typename T,
  typename Alloc = std::allocator<typename std::remove_extent<T>::type>
>
class ts_ptr {
 public:
  constexpr static bool is_array = std::is_array<T>::value;
  using type = typename std::remove_extent<T>::type;
  using type_ptr = type*;

 private:
  Alloc alloc_;
  uintmax_t size_ = 0;
  type_ptr ptr_ = nullptr;
  uintmax_t *count_ = nullptr;
  std::mutex *mutex_ = nullptr;

  constexpr void init_storage (void) {
    this->count_ = new uintmax_t(1);
    this->mutex_ = new std::mutex();
  }

  constexpr void free (void) {
    this->alloc_.deallocate(this->ptr_, this->size());

    delete this->count_;

    this->mutex_->unlock();
    delete this->mutex_;
  }

  constexpr void dec (void) {
    if (this->ptr_ != nullptr) {
      this->mutex_->lock();

      if (--(*this->count_) == 0) {
        this->free();
      } else {
        this->mutex_->unlock();
      }

      this->size_ = 0;
      this->ptr_ = nullptr;
      this->count_ = nullptr;
      this->mutex_ = nullptr;
    }
  }

  constexpr void inc (void) {
    if (this->ptr_ != nullptr) {
      this->mutex_->lock();
      ++(*this->count_);
      this->mutex_->unlock();
    }
  }

 public:

  constexpr void copy_meta (ts_ptr const& ot) {
    this->ptr_ = ot.ptr_;
    this->count_ = ot.count_;
    this->mutex_ = ot.mutex_;
    this->size_ = ot.size_;
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
    this->size_ = size;

    this->ptr_ = this->alloc_.allocate(size);

    for (uintmax_t i = 0; i < size; ++i) {
      new (this->ptr_ + i) type();
    }

    this->init_storage();
  }

  template <typename X = void, typename = std::enable_if_t<!is_array, X>, typename... ARGS>
  constexpr void allocate (ARGS&&... args) {
    this->dec();
    this->size_ = 1;

    this->ptr_ = this->alloc_.allocate(1);
    *this->ptr_ = type(std::forward<ARGS>(args)...);

    this->init_storage();
  }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr begin (void) { return this->ptr_; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr end (void) { return this->ptr_ + this->size(); }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const begin (void) const { return this->ptr_; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const end (void) const { return this->ptr_ + this->size(); }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const cbegin (void) const { return this->ptr_; }

  template <typename X = void, typename = std::enable_if_t<is_array, X>>
  constexpr type_ptr const cend (void) const { return this->ptr_ + this->size(); }

  constexpr type_ptr get (void) { return this->ptr_; }
  constexpr type_ptr const get (void) const { return this->ptr_; }

  constexpr void release (void) { this->dec(); }

  template <typename I, typename = std::enable_if_t<is_array, I>>
  constexpr type& operator [] (I pos) { return this->ptr_[pos]; }

  template <typename I, typename = std::enable_if_t<is_array, I>>
  constexpr type const& operator [] (I pos) const { return this->ptr_[pos]; }

  constexpr type const& operator * (void) const { return *this->ptr_; }
  constexpr type& operator * (void) { return *this->ptr_; }

  constexpr uintmax_t const& size (void) const { return this->size_; }
  constexpr bool empty (void) const { return this->size_ == 0; }

  constexpr uintmax_t const& count (void) const { return *this->count_; }

  constexpr operator bool (void) const { return this->ptr_ != nullptr; }
};
