#pragma once

#include "base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class genetic : public __EVO_BASE {
   public:
    __EVO_USING_TYPES(genetic);
    __EVO_USING_FUNCTIONS;

   protected:
    siz_t _popsize;
    siz_t _elitism = 1;

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::initialize(chr, fit, this->popsize());
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::evolve(chr, fit, this->popsize() - this->elitism());
    }

    siz_t select (chr_t* chr, fit_t* fit, siz_t old, siz_t all) override {
      this->partition_best(chr, fit, old, this->elitism());

      for (siz_t i = this->elitism(), j = old; i < this->popsize(); ++i, ++j) {
        chr[i] = std::move(chr[j]);
        fit[i] = std::move(fit[j]);
      }

      return this->popsize();
    }

   public:
    genetic (siz_t size, sed_t seed = 0)
      : evo_t(size + size, seed), _popsize(size) {}

    evo_t* copy (void) const { return new gen_t(*this); }

    siz_t popsize (void) const { return this->_popsize; }

    siz_t elitism (void) const { return this->_elitism; }
    siz_t& elitism (void) { return this->_elitism; }
  };

};
