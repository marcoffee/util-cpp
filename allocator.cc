#include <atomic>
#include "atomic.hh"
#include "allocator.hh"

namespace util::allocator {

  static std::atomic_uintmax_t _peak = 0, _total = 0, _freed = 0;

  void add_memory (uintmax_t const size) {
    _total += size;
    util::atomic::max(_peak, now());
  }

  void free_memory (uintmax_t const size) {
    _freed += size;
  }

  uintmax_t peak (void) { return _peak.load(std::memory_order_relaxed); }
  uintmax_t total (void) { return _total.load(std::memory_order_relaxed); }
  uintmax_t freed (void) { return _freed.load(std::memory_order_relaxed); }
  uintmax_t now (void) { return total() - freed(); }
}
