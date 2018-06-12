#include "base.hh"
#include "ts_ptr.hh"

namespace memory_stats {
  static std::atomic_uintmax_t _now = 0, _peak = 0, _total = 0;
  constexpr auto const& mem_order = std::memory_order_relaxed;

  uintmax_t now (void) { return _now.load(mem_order); }
  uintmax_t peak (void) { return _peak.load(mem_order); }
  uintmax_t total (void) { return _total.load(mem_order); }

  void update_max (void) {
    util::atomic_max(_peak, now());
  }

  void add_memory (uintmax_t count) {
    _now += count;
    _total += count;
    update_max();
  }

  void del_memory (uintmax_t count) {
    _now -= count;
  }

};
