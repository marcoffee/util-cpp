#pragma once

#include "base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class clonalg : public __EVO_BASE {
   public:
    __EVO_USING_TYPES(clonalg);
    __EVO_USING_FUNCTIONS;

    using clonal = typename generators::clonal;
    using mutation_counter = typename clonal::mutation_counter;

   protected:
    siz_t _popsize;
    siz_t _cs;
    bool _drift = true;

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::initialize(chr, fit, this->popsize());
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t space) override {
      return this->evo_t::evolve(chr, fit, this->popsize() * this->cs());
    }

    siz_t select (chr_t* chr, fit_t* fit, siz_t old, siz_t all) override {
      for (siz_t i = 0; i < this->popsize(); ++i) {
        siz_t const start = this->popsize() + (i * this->cs());
        siz_t const best_child = this->find_best(fit + start, this->cs()) + start;

        bool const is_better = (
          this->neutral_drift() ?
            !this->compare(fit[i], fit[best_child]) :
             this->compare(fit[best_child], fit[i])
        );

        if (is_better) {
          std::swap(chr[i], chr[best_child]);
          std::swap(fit[i], fit[best_child]);
        }
      }

      return this->popsize();
    }

   public:
    clonalg (siz_t popsize, siz_t cs, sed_t seed = 0)
      : evo_t(popsize * (cs + 1), seed), _popsize(popsize), _cs(cs) {}

    evo_t* copy (void) const { return new gen_t(*this); }

    void set_mutator (
      mutator const& mutate,
      mutation_counter const& count, bool rank_based
    ) {
      this->set_generator(clonal(
        mutate, this->cs(),
        rank_based ? this->make_index_comparator() : nullptr,
        count
      ));
    }

    siz_t popsize (void) const { return this->_popsize; }
    siz_t cs (void) const { return this->_cs; }

    bool& neutral_drift (void) { return this->_drift; }
    bool const& neutral_drift (void) const { return this->_drift; }
  };

};
