#pragma once

#include <memory>
#include <iostream>

namespace util::allocator {

  void add_memory (uintmax_t amount);
  void free_memory (uintmax_t amount);

  uintmax_t peak (void);
  uintmax_t total (void);
  uintmax_t freed (void);
  uintmax_t now (void);

  template <typename T>
  class allocator {
  public:
    using base = std::allocator<T>;

  private:
    base _alloc;

  public:
    using value_type = typename base::value_type;
    using size_type = typename base::size_type;
    using difference_type = typename base::difference_type;
    using propagate_on_container_move_assignment = typename base::propagate_on_container_move_assignment;
    using is_always_equal = typename base::is_always_equal;

    template <typename... ARGS>
    allocator (ARGS&&... args) : _alloc(std::forward<ARGS>(args)...) {}

    template <typename... ARGS>
    [[nodiscard]] T* allocate (std::size_t n, ARGS&&... args) {
      // Log allocation
      add_memory(n * sizeof(T));
      // Calls base allocator
      return this->_alloc.allocate(n, std::forward<ARGS>(args)...);
    }

    template <typename... ARGS>
    void deallocate (T* p, std::size_t n, ARGS&&... args) {
      // Log deallocation
      free_memory(n * sizeof(T));
      // Calls base deallocator
      this->_alloc.deallocate(p, n, std::forward<ARGS>(args)...);
    }

    bool operator == (allocator const& ot) const {
      return this->_alloc == ot._alloc;
    }

    bool operator != (allocator const& ot) const {
      return this->_alloc != ot._alloc;
    }

  };

};
