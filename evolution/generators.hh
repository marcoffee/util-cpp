#pragma once

#include "macros.hh"

namespace __EVO_NAMESPACE {

  template <typename EVO>
  class base_generators {
   public:
    __EVO_COPY_TYPES(base_generators, typename EVO);
    __EVO_USING_FUNCTIONS;

    static generator always_cross (
      generator const& cross, mutator const& mutate, double m_prob
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
      generator const& cross, generator const& mutate, double m_prob
    ) {
      std::uniform_real_distribution<double> dist;

      return [ cross, mutate, m_prob, &dist ] (evo_t& evo) -> chr_v {
        if (dist(evo.random()) > m_prob) {
          return cross(evo);
        }

        return mutate(evo);
      };
    }

    class clonal {
   public:
      using mutation_counter = std::function<siz_t(evo_t&, siz_t, siz_t)>;

      static siz_t ranking_count (evo_t&, siz_t, siz_t rank) {
        return rank + 1;
      }

      static mutation_counter constant_count (siz_t number) {
        return [ number ] (evo_t&, siz_t, siz_t) {
          return number;
        };
      }

   private:
      mutator mutate_;
      siz_t cs_;
      index_comparator cmp_;
      mutation_counter count_;

   public:
      clonal (
        mutator const& mutate, siz_t cs, index_comparator const& cmp,
        mutation_counter const& count = ranking_count
      ) : mutate_{ mutate }, cs_{ cs }, cmp_{ cmp }, count_{ count } {}

      clonal (
        mutator const& mutate, siz_t cs,
        mutation_counter const& count = ranking_count
      ) : clonal{ mutate, cs, nullptr, count } {}

      chr_v operator () (evo_t& evo) {
        siz_t const size = evo.size();
        chr_v result;
        siz_t *idx = new siz_t[size];

        std::iota(idx, idx + size, 0);
        result.reserve(size * this->cs_);

        if (this->cmp_) {
          std::stable_sort(idx, idx + size, this->cmp_);
        }

        for (siz_t i = 0; i < size; ++i) {
          siz_t const pos = idx[i];
          siz_t const start = result.size();
          siz_t const mutations = this->count_(evo, pos, i);

          std::fill_n(std::back_inserter(result), this->cs_, evo.chr_at(pos));

          for (siz_t j = 0; j < this->cs_; ++j) {
            chr_t& chr = result[start + j];

            for (siz_t k = 0; k < mutations; ++k) {
              chr = std::move(this->mutate_(evo, chr).front());
            }
          }
        }

        delete[] idx;
        return result;
      }

    };

  };

};
