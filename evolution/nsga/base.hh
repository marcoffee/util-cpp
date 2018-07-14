#pragma once

#include "../base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class nsga : public __BASE_CLASS {

  public:
    __EVO_USING_TYPES;
    using gen_t = nsga;

    __EVO_USING_FUNCTIONS;

  protected:
    siz_t _popsize;
    siz_t _children;
    siz_t *_fro;
    dis_t *_dis;

    bool dominates (fit_t const& fa, fit_t const& fb, bool& a_d, bool& b_d) {
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

      a_d = a_dom;
      b_d = b_dom;

      return a_dom ^ b_dom;
    }

    siz_t nd_sorting (
      fit_t const* fit, siz_t* ranks, siz_t* ranked, siz_t* ends,
      siz_t size, siz_t fill
    ) {
      if (fill == 0) {
        return 0;
      }

      siz_v* dominated = new siz_v[size]();
      siz_t* count = new siz_t[size]();
      siz_t* queue = ranked;
      siz_t* start = ranked;

      for (siz_t i = 0; i < size; ++i) {
        bool i_d, j_d;

        for (siz_t j = i + 1; j < size; ++j) {
          if (this->dominates(fit[i], fit[j], i_d, j_d)) {
            if (i_d) {
              dominated[i].emplace_back(j);
              // std::cout << i << " dominates " << j << std::endl;
              count[j]++;
            } else {
              dominated[j].emplace_back(i);
              // std::cout << j << " dominates " << i << std::endl;
              count[i]++;
            }
          }
        }

        if (count[i] == 0) {
          ranks[i] = 0;
          *(ranked++) = i;
        }
      }

      siz_t fronts = 0;
      siz_t found = 0;
      bool stop = false;

      while (queue != ranked && !stop) {
        siz_t const pos = *(queue++);

        for (siz_t const dom : dominated[pos]) {
          if (--count[dom] != 0) {
            continue;
          }

          ranks[dom] = ranks[pos] + 1;

          if (ranks[dom] != fronts) {
            found += ranked - start;
            ends[fronts++] = found;
            start = ranked;
            stop = found >= fill;
          }

          *(ranked++) = dom;

          if (stop) {
            break;
          }
        }
      }

      if (!stop) {
        found += ranked - start;
        ends[fronts++] = found;
      }

      delete[] count;
      delete[] dominated;

      return fronts;
    }

    void cd_sorting (fit_t const* fit, dis_t* distance, siz_t* order, siz_t size) {
      siz_t const last = size - 1;
      siz_t* idx = new siz_t[size];

      for (siz_t i = 0; i < size; ++i) {
        distance[order[i]] = 0.0;
      }

      for (siz_t i = 0; i < this->dimensions(); ++i) {
        std::copy(order, order + size, idx);
        std::stable_sort(idx, idx + size, this->build_compare(fit, i));

        siz_t const id_a = idx[0], id_b = idx[last];
        dis_t const delta = this->subtract(fit[id_a], fit[id_b], i);

        distance[id_a] = std::numeric_limits<dis_t>::infinity();
        distance[id_b] = std::numeric_limits<dis_t>::infinity();

        for (siz_t j = 1; j < last; ++j) {
          distance[idx[j]] += delta * this->subtract(
            fit[idx[j - 1]], fit[idx[j + 1]], i
          );
        }

      }

      delete[] idx;
    }

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::initialize(chr, fit, this->popsize());
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::evolve(chr, fit, this->children());
    }

    siz_t select (chr_t* chr, fit_t* fit, siz_t, siz_t all) override {
      siz_t* ranked = new siz_t[all];
      siz_t* ends = new siz_t[all];

      siz_t const fronts = this->nd_sorting(fit, this->_fro, ranked, ends, all, this->popsize());
      siz_t const found = ends[fronts - 1];
      siz_t start = 0;

      std::fill(this->_dis, this->_dis + found, -std::numeric_limits<dis_t>::infinity());

      for (siz_t i = 0; i < fronts; ++i) {
        this->cd_sorting(fit, this->_dis, ranked + start, ends[i] - start);
        start = ends[i];
      }

      if (found > this->popsize()) {
        siz_t const conti = ends[fronts - 2];
        siz_t const rem = this->popsize() - conti;
        std::function<bool(siz_t, siz_t)> const cmp{
          [ dis = this->_dis ] (siz_t a, siz_t b) { return dis[b] > dis[a]; }
        };

        util::iterator::multipartition(cmp,
          ranked + conti, ranked + found, rem, ranked + conti
        );
      }

      util::iterator::multiorder(
        ranked, ranked + this->popsize(), 0, chr, fit, this->_dis, this->_fro
      );

      delete[] ends;
      delete[] ranked;
      return this->popsize();
    }

  public:

    nsga (siz_t popsize, siz_t children, siz_t dimensions, siz_t seed)
      : evo_t(dimensions, popsize + children, seed),
        _popsize(popsize), _children(children) {}

    nsga (siz_t popsize, siz_t dimensions, siz_t seed)
      : nsga(popsize, popsize, dimensions, seed) {}

    evo_t* copy (void) const override { return new nsga(*this); }

    siz_t const& popsize (void) const { return this->_popsize; }
    siz_t const& children (void) const { return this->_children; }

    siz_t test_nd_sorting (
      fit_t const* fit, siz_t* ranks, siz_t* ranked, siz_t* ends,
      siz_t size, siz_t fill
    ) {
      return this->nd_sorting(fit, ranks, ranked, ends, size, fill);
    }

    void test_cd_sorting (
      fit_t const* fit, dis_t* distance, siz_t* order, siz_t size
    ) {
      return this->cd_sorting(fit, distance, order, size);
    }

  };

};
