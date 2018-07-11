#pragma once

#include "base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class genetic : public __BASE_CLASS {
  public:
    __EVO_USING_TYPES;
    using gen_t       = genetic;

    __EVO_USING_FUNCTIONS;
    using mutator     = std::function<chr_v(evo_t&, chr_t const& chr)>;
    using crossover   = std::function<chr_v(evo_t&)>;

  protected:
    siz_t _popsize;
    siz_t _elitism = 1;
    double _m_prob;

    crossover _cross = nullptr;
    mutator _mutate = nullptr;

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      for (siz_t i = 0; i < this->popsize(); ++i) {
        chr[i] = this->create();
        fit[i] = this->evaluate(chr[i]);
      }

      return this->popsize();
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t) override {
      std::uniform_real_distribution<double> dist;

      for (siz_t i = 0; i < this->popsize(); ) {
        chr_v off;

        for (chr_t& child : this->cross()) {
          if (dist(this->random()) > this->_m_prob) {
            off.emplace_back(std::move(child));
            continue;
          }

          chr_v mutation = this->mutate(child);
          std::move(mutation.begin(), mutation.end(), std::back_inserter(off));
        }

        for (chr_t& child : off) {
          chr[i] = std::move(child);
          fit[i] = this->evaluate(chr[i]);

          if (++i >= this->popsize()) {
            break;
          }
        }
      }

      return this->popsize();
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
    genetic (siz_t size, double m_prob, sed_t seed = 0)
      : evo_t(size + size, seed), _popsize(size), _m_prob(m_prob) {}

    evo_t* copy (void) const { return new gen_t(*this); }

    void set_crossover (crossover const& cross) { this->_cross = cross; }
    void set_mutator (mutator const& mutate) { this->_mutate = mutate; }

    chr_v cross (void) { return this->_cross(*this); }
    chr_v mutate (chr_t const& chr) { return this->_mutate(*this, chr); }

    siz_t popsize (void) const { return this->_popsize; }

    siz_t elitism (void) const { return this->_elitism; }
    siz_t& elitism (void) { return this->_elitism; }

    double m_prob (void) const { return this->_m_prob; }
    double& m_prob (void) { return this->_m_prob; }
  };

};
