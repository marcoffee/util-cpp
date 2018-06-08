#include "ts_ptr.hh"

namespace memory_stats {
  static std::atomic_uintmax_t _now = 0, _peak = 0, _total = 0;
  constexpr auto const& mem_order = std::memory_order_relaxed;

  void update_max (void) {
    uintmax_t val = _now.load(mem_order);

    for (
      uintmax_t max_val = _peak.load(mem_order);
      max_val < val && !_peak.compare_exchange_weak(max_val, val, mem_order);
      val = _now.load(mem_order)
    ) {};
  }

  void add_memory (uintmax_t count) {
    _now += count;
    _total += count;
    update_max();
  }

  void del_memory (uintmax_t count) {
    _now -= count;
  }

  uintmax_t now (void) { return _now.load(mem_order); }
  uintmax_t peak (void) { return _peak.load(mem_order); }
  uintmax_t total (void) { return _total.load(mem_order); }

};
