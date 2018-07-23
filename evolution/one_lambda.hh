#pragma once

#include "base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class one_lambda : public __EVO_BASE {
  public:
    __EVO_USING_TYPES(one_lambda);
    __EVO_USING_FUNCTIONS;

  private:
    uintmax_t _lambda;
    bool _drift = true;
    bool _changed = false;

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      this->_changed = true;
      return this->evo_t::initialize(chr, fit, 1);
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::evolve(chr, fit, this->lambda());
    }

    siz_t select (chr_t* chr, fit_t* fit, siz_t, siz_t) override {
      uintmax_t best_child = this->find_best(fit + 1, this->lambda()) + 1;
      bool const is_better = (
        this->neutral_drift() ?
          !this->compare(fit[0], fit[best_child]) :
           this->compare(fit[best_child], fit[0])
      );

      if (is_better) {
        std::swap(chr[0], chr[best_child]);
        std::swap(fit[0], fit[best_child]);
        this->_changed = true;
      }

      return 1;
    }

  public:

    one_lambda (uintmax_t lambda, uintmax_t seed)
      : evo_t(1 + lambda, seed), _lambda(lambda) {};

    evo_t* copy (void) const override { return new one_lambda(*this); }
    siz_t best (siz_t = 0) override { return 0; }

    void set_mutator (mutator const& mutate) {
      this->set_generator([ mutate ] (evo_t& evo) {
        return mutate(evo, evo.chr_at(0));
      });
    }

    uintmax_t& lambda (void) { return this->_lambda; }
    uintmax_t const& lambda (void) const { return this->_lambda; }

    bool& neutral_drift (void) { return this->_drift; }
    bool const& neutral_drift (void) const { return this->_drift; }

    bool const& changed (void) const { return this->_changed; }
  };

};
