#include <atomic>
#include "atomic.hh"
#include "allocator.hh"

namespace util::allocator {

  static std::atomic_uintmax_t peak_ = 0, total_ = 0, freed_ = 0;

  // Adds to total memory allocated
  void add_memory (uintmax_t const size) {
    total_ += size;
    util::atomic::max(peak_, now());
  }

  // Adds to total memory freed
  void free_memory (uintmax_t const size) {
    freed_ += size;
  }

  uintmax_t peak (void) { return peak_.load(std::memory_order_relaxed); }
  uintmax_t total (void) { return total_.load(std::memory_order_relaxed); }
  uintmax_t freed (void) { return freed_.load(std::memory_order_relaxed); }
  uintmax_t now (void) { return total() - freed(); }
}
