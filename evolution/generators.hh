#pragma once

#include "base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class generators : public __BASE_CLASS {
  public:
    __EVO_USING_TYPES;
    __EVO_USING_FUNCTIONS;
    __EVO_USING_GENERATOR_FUNCTIONS;

    static generator always_cross (
      crossover const& cross, single_mutator const& mutate, double m_prob
    ) {
      std::uniform_real_distribution<double> dist;

      return [ cross, mutate, m_prob, &dist ] (evo_t& evo) -> chr_v {
        chr_v off;

        for (chr_t& child : cross(evo)) {
          if (dist(evo.random()) > m_prob) {
            off.emplace_back(std::move(child));
            continue;
          }

          chr_v mutation = mutate(evo, child);
          std::move(mutation.begin(), mutation.end(), std::back_inserter(off));
        }

        return off;
      };
    }

    static generator cross_or_mutate (
      crossover const& cross, mutator const& mutate, double m_prob
    ) {
      std::uniform_real_distribution<double> dist;

      return [ cross, mutate, m_prob, &dist ] (evo_t& evo) -> chr_v {
        if (dist(evo.random()) > m_prob) {
          return cross(evo);
        }

        return mutate(evo);
      };
    }

    static generator mutate_only (mutator const& mutate) {
      return [ mutate ] (evo_t& evo) { return mutate(evo); };
    }

    static generator crossover_only (crossover const& cross) {
      return [ cross ] (evo_t& evo) { return cross(evo); };
    }

  };

};
