#pragma once

#include "../base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class nsga : __BASE_CLASS {

  public:
    __EVO_USING_TYPES;
    using gen_t = nsga;

    __EVO_USING_FUNCTIONS;

  protected:
    siz_t _popsize;
    difference* _dif;

    bool dominates (fit_t const& fa, fit_t const& fb, bool* a_d, bool* b_d) {
      bool a_dom = false;
      bool b_dom = false;

      for (siz_t i = 0; i < this->dimensions(); ++i) {
        if (this->compare(fa, fb, i)) {
          a_dom = true;

          if (b_dom) {
            break;
          }

        } else if (this->compare(fb, fa, i)) {
          b_dom = true;

          if (a_dom) {
            break;
          }
        }
      }

      *a_d = a_dom;
      *b_d = b_dom;

      return a_dom ^ b_dom;
    }

    void nd_sorting (fit_t const* fit, siz_t* ranks, siz_t size) {
      siz_v* dominated = new siz_v[size]();
      siz_t* count = new siz_t[size]();
      siz_t* sorted = new siz_t[size];
      siz_t* queue = sorted;

      for (siz_t i = 0; i < size; ++i) {
        bool i_d, j_d;

        for (siz_t j = i + 1; j < size; ++j) {
          if (dominates(fit[i], fit[j], i_d, j_d)) {
            if (i_d) {
              dominated[i].emplace_back(j);
              count[j]++;
            } else {
              dominated[j].emplace_back(i);
              count[i]++;
            }
          }
        }

        if (count[i] == 0) {
          *(sorted++) = i;
          ranks[i] = 0;
        }
      }

      while (queue != sorted) {
        siz_t chr = *(queue++);

        for (siz_t dom : dominated[chr]) {
          if (--count[dom] == 0) {
            ranks[dom] = ranks[chr] + 1;
            *(sorted++) = dom;
          }
        }
      }

      delete[] sorted;
      delete[] count;
      delete[] dominated;
    }

    void cd_sorting (fit_t const* fit, double* distance, siz_t size) {
      siz_t const last = size - 1;
      siz_t* idx = new siz_t[size];

      std::fill(distance, distance + size, 0.0);

      for (uintmax_t i = 0; i < this->dimensions(); ++i) {
        std::iota(idx, idx + size, 0);
        std::stable_sort(idx, idx + size, this->build_compare(fit, i));

        siz_t const id_a = idx[0], id_b = idx[last];
        double const delta = 1.0 / this->difference(fit[id_a], fit[id_b], i);

        distance[id_a] = std::numeric_limits<double>::infinity();
        distance[id_b] = std::numeric_limits<double>::infinity();

        for (siz_t j = 1; j < last; ++j) {
          distance[idx[j]] += delta * this->difference(
            fit[idx[j - 1]], fit[idx[j + 1]], i
          );
        }

      }

      delete[] idx;
    }



  };

};
