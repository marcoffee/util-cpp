#pragma once

#include "base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class one_lambda : public __BASE_CLASS {
  public:
    __EVO_USING_TYPES;
    using onl_t       = one_lambda;

    __EVO_USING_FUNCTIONS;
    using mutator     = std::function<chr_v(evo_t&, chr_t const& chr)>;

  private:
    mutator _mutate = nullptr;
    uintmax_t _lambda;
    bool _drift = true;
    bool _changed = false;

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      chr[0] = this->create();
      fit[0] = this->evaluate(chr[0]);
      this->_changed = true;
      return 1;
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t) override {
      chr_t const& best = this->chr_at(0);

      for (siz_t i = 0; i < this->lambda(); ) {
        for (chr_t& child : this->mutate(best)) {
          fit[i] = this->evaluate(child);
          chr[i] = std::move(child);

          if (++i >= this->lambda()) {
            break;
          }
        }
      }

      return this->lambda();
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

    void set_mutator (mutator const& mutate) { this->_mutate = mutate; }
    chr_v mutate (chr_t const& chr) { return this->_mutate(*this, chr); }

    uintmax_t& lambda (void) { return this->_lambda; }
    uintmax_t const& lambda (void) const { return this->_lambda; }

    bool& neutral_drift (void) { return this->_drift; }
    bool const& neutral_drift (void) const { return this->_drift; }

    bool const& changed (void) const { return this->_changed; }
  };

};
