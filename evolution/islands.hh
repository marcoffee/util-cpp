#pragma once

#include "base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class islands {
   public:
    __EVO_USING_TYPES(islands);
    __EVO_USING_FUNCTIONS;

   private:
    rnd_t rnd_;
    siz_t size_ = 0;
    evo_t** world_ = nullptr;

    void alloc (bool) { this->world_ = new evo_t*[this->size()](); }
    void release (bool) { this->size_ = 0; this->world_ = nullptr; }

    void free (bool) {
      if (this->world_ == nullptr) {
        return;
      }

      for (siz_t i = 0; i < this->size(); ++i) {
        delete this->world_[i];
      }

      delete[] this->world_;
      this->release(false);
    }

    void copy_meta (islands const& ot, bool) {
      this->size_ = ot.size_;
    }

    islands& copy_from (islands const& ot, bool) {
      bool const diff = this->size() != ot.size();

      if (diff) {
        this->free(false);
      }

      this->copy_meta(ot, false);

      if (diff) {
        this->alloc(false);
      }

      for (siz_t i = 0; i < this->size(); ++i) {
        if (ot.world_[i] != nullptr) {
          this->world_[i] = ot.world_[i]->copy();
        }
      }

      this->rnd_ = ot.rnd_;
    }

    islands& move_from (islands&& ot) {
      this->free();
      this->copy_meta(ot, false);
      this->world_ = ot.world_;
      this->rnd_ = std::move(ot.rnd_);
      ot.release(false);
    }

   public:
    explicit islands (siz_t size, siz_t seed = 0)
    : rnd_{ seed }, size_{ size } { this->alloc(false); }

    islands (islands const& ot) { this->copy_from(ot, false); }
    islands (islands&& ot) { this->move_from(std::move(ot), false); }

    islands& operator = (islands const& ot) {
      return this->copy_from(ot, false);
    }

    islands& operator = (islands&& ot) {
      return this->move_from(std::move(ot), false);
    }

    inline rnd_t& random (void) { return this->rnd_; }

    template <
      typename EVO,
      typename... ARGS,
      typename = std::enable_if_t<std::is_base_of_v<evo_t, EVO>>
    >
    void emplace_at (siz_t i, ARGS&&... args) {
      delete this->world_[i];
      this->world_[i] = new EVO(std::forward<ARGS>(args)...);
    }

    void step (bool parallel = true) {
      IF_OMP(parallel for schedule(static) if(parallel))
      for (siz_t i = 0; i < this->size(); ++i) {
        this->world_[i]->step();
      }
    }

    void migrate_move (siz_t amount, bool parallel = true) {
      if (this->size() == 1) {
        return;
      }

      siz_t mig_total = 0;
      siz_t* mig_offs = new siz_t[this->size()];

      for (siz_t i = 0; i < this->size(); ++i) {
        siz_t const wld_size = this->world(i).size();
        siz_t const mig_size = std::min(amount, wld_size);

        mig_offs[i] = mig_total;
        mig_total += mig_size;
      }

      chr_t* chr = new chr_t[mig_total];
      fit_t* fit = new fit_t[mig_total];

      IF_OMP(parallel for schedule(static) if(parallel))
      for (siz_t i = 0; i < this->size(); ++i) {
        siz_t const wld_size = this->world(i).size();
        siz_t const mig_size = std::min(amount, wld_size);
        siz_t const mig_off = mig_offs[i];

        siz_t* idx = new siz_t[wld_size];

        std::iota(idx, idx + wld_size, 0);
        std::shuffle(idx, idx + wld_size, this->world(i).random());
        std::sort(idx, idx + mig_size, std::greater<siz_t>());

        for (siz_t j = 0; j < mig_size; ++j) {
          this->world(i).remove(idx[j], chr[mig_off + j], fit[mig_off + j]);
        }

        delete[] idx;
      }

      util::iterator::multishuffle(mig_total, this->random(), chr, fit);

      IF_OMP(parallel for schedule(static) if(parallel))
      for (siz_t i = 0; i < this->size(); ++i) {
        siz_t const wld_size = this->world(i).size();
        siz_t const mig_size = std::min(amount, wld_size);
        siz_t const mig_off = mig_offs[i];

        for (siz_t j = mig_off, stop = mig_off + mig_size; j < stop; ++j) {
          this->world(i).add(std::move(chr[j]), std::move(fit[j]));
        }
      }

      delete[] mig_offs;
      delete[] chr;
      delete[] fit;
    }

    siz_t size (void) const { return this->size_; }

    template <
      typename EVO = evo_t,
      typename = std::enable_if_t<std::is_base_of_v<evo_t, EVO>>,
      typename = std::enable_if_t<!std::is_const_v<EVO>>
    >
    EVO& world (siz_t i) {
      if constexpr (std::is_same_v<EVO, evo_t>) {
        return *this->world_[i];
      }

      return *dynamic_cast<EVO*>(this->world_[i]);
    }

    template <
      typename EVO = evo_t,
      typename = std::enable_if_t<std::is_base_of_v<evo_t, EVO>>
    >
    EVO const& world (siz_t i) const {
      if constexpr (std::is_same_v<EVO, evo_t>) {
        return *this->world_[i];
      }

      return *dynamic_cast<EVO const*>(this->world_[i]);
    }

    evo_t& operator [] (siz_t i) { return this->world(i); }
    evo_t const& operator [] (siz_t i) const { return this->world(i); }
  };

};
