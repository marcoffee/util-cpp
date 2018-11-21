#pragma once

#include <cstdint>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <tuple>

#include "util_constexpr.hh"
#include "pointer.hh"


namespace util {

  // Class to estimate time of completing
  template <typename FLT = double>
  class eta {
   public:
    using siz_t = uintmax_t;
    using flt_t = FLT;

    using duration = std::chrono::duration<flt_t>;
    using clock = std::chrono::high_resolution_clock;

    // Gets the time since the linux epoch in seconds
    static flt_t time_since_epoch (void) {
      return duration{ clock::now().time_since_epoch() }.count();
    }

   private:
    siz_t head_ = 0, tail_ = 0, max_size_ = 0;
    flt_t* times_ = nullptr;
    flt_t* percs_ = nullptr;

    // Release eta's members
    void release (void) {
      this->head_ = this->tail_ = this->max_size_ = 0;
      this->times_ = nullptr;
      this->percs_ = nullptr;
    }

    // Free eta's memory
    void free (void) {
      delete[] this->times_;
      delete[] this->percs_;
      this->release();
    }

    // Copy metadata from another eta
    void copy_meta (eta const& ot) {
      this->head_ = ot.head_;
      this->tail_ = ot.tail_;
      this->max_size_ = ot.max_size_;
    }

    // Copy data from another eta
    eta& copy_from (eta const& ot) {
      if (this != &ot) {
        this->free();
        this->copy_meta(ot);
        this->times_ = pointer::duplicate(ot.times_, ot.max_size_);
        this->percs_ = pointer::duplicate(ot.percs_, ot.max_size_);
      }

      return *this;
    }

    // Move data from another eta
    eta& move_from (eta& ot) {
      if (this != &ot) {
        this->free();
        this->copy_meta(ot);
        this->times_ = std::move(ot.times_);
        this->percs_ = std::move(ot.percs_);
        ot.release();
      }

      return *this;
    }

   public:
    // Eta's constructors and assignment operators
    constexpr eta (void) {}

    explicit eta (siz_t max_size)
    : max_size_{ max_size }
    , times_{ new flt_t[max_size] }
    , percs_{ new flt_t[max_size] } {}

    eta (siz_t max_size, flt_t perc)
    : eta{ max_size } {
      this->add(perc);
    }

    eta (eta const& ot) { this->copy_from(ot); }
    eta (eta&& ot) { this->move_from(ot); }

    eta& operator = (eta const& ot) { return this->copy_from(ot); }
    eta& operator = (eta&& ot) { return this->move_from(ot); }

    // eta's destructor
    ~eta (void) { this->free(); }
    void destroy (void) { this->free(); }

    // Reset an eta to its initial state (empty)
    void reset (void) {
      this->head_ = this->tail_ = 0;
    }

    // Reset an eta to its initial state (with an initial percentage)
    void reset (flt_t perc) {
      this->reset();
      this->add(perc);
    }

    // Add time to eta
    flt_t add (flt_t perc) {
      flt_t time;
      // Get estimation
      flt_t const eta_time = this->get(perc, time);

      // Save time
      this->times_[this->tail_] = time;
      this->percs_[this->tail_] = perc;

      // Move tail
      this->tail_ = (this->tail_ + 1) % this->max_size_;

      if (this->head_ == this->tail_) {
        // Move head
        this->head_ = (this->head_ + 1) % this->max_size_;
      }

      return eta_time;
    }

    // Get time estimation
    flt_t get (flt_t perc, flt_t& time_point = defref<flt_t>) const {
      flt_t const time = eta::time_since_epoch();
      flt_t const time_0 = this->times_[this->head_];
      flt_t const perc_0 = this->percs_[this->head_];

      time_point = time;
      return ((1.0 - perc) * (time - time_0)) / (perc - perc_0);
    }

    auto add (siz_t done, siz_t total) {
      return this->add(flt_t(done) / total);
    }

    auto get (siz_t done, siz_t total) const {
      return this->get(flt_t(done) / total);
    }

    template <typename... ARGS>
    auto operator () (ARGS&&... args) {
      return this->add(std::forward<ARGS>(args)...);
    }
  };
}
