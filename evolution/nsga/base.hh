#pragma once

#include <filesystem>
#include <fstream>
#include "../../string.hh"
#include "../base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class nsga : public __EVO_BASE {

   public:
    __EVO_USING_TYPES(nsga);
    __EVO_USING_FUNCTIONS;

    using const_fro_iterator = siz_t const*;
    using const_dis_iterator = dis_t const*;

   protected:
    siz_t popsize_;
    siz_t children_;
    siz_t fronts_;
    siz_t *fro_;
    dis_t *dis_;

    void alloc (bool parent) override {
      if (parent) {
        this->evo_t::alloc(true);
      }

      this->fro_ = new siz_t[this->max_size()];
      this->dis_ = new dis_t[this->max_size()];
    }

    void release (bool parent) override {
      if (parent) {
        this->evo_t::release(true);
      }

      this->fro_ = nullptr;
      this->dis_ = nullptr;
    }

    void free (bool parent) override {
      if (parent) {
        this->evo_t::free(true);
      }

      delete[] this->fro_;
      delete[] this->dis_;
      this->release(false);
    }

    void copy_meta (nsga const& ot, bool parent) {
      if (parent) {
        this->evo_t::copy_meta(ot, true);
      }

      this->popsize_ = ot.popsize_;
      this->children_ = ot.children_;
      this->fronts_ = ot.fronts_;
    }

    nsga& copy_from (nsga const& ot, bool parent) {
      bool const dif = this->max_size() != ot.max_size();

      if (parent) {
        this->evo_t::copy_from(ot, true);
      }

      if (dif) {
        this->free(false);
        this->copy_meta(ot, false);
        this->alloc(false);
      }

      std::copy(ot.fro_, ot.fro_ + ot.max_size(), this->fro_);
      std::copy(ot.dis_, ot.dis_ + ot.max_size(), this->dis_);

      return *this;
    }

    nsga& move_from (nsga& ot, bool parent) {
      if (parent) {
        this->evo_t::move_from(ot, true);
      }

      this->free(false);
      this->copy_meta(ot, false);

      this->fro_ = std::move(ot.fro_);
      this->dis_ = std::move(ot.dis_);
      ot.release(true);

      return *this;
    }

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
              count[j]++;
            } else {
              dominated[j].emplace_back(i);
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

      while (queue != ranked and !stop) {
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
        std::stable_sort(idx, idx + size, this->make_index_comparator(fit, i));

        siz_t const id_a = idx[0], id_b = idx[last];
        dis_t const delta = 1.0 / this->subtract(fit[id_b], fit[id_a], i);

        distance[id_a] = std::numeric_limits<dis_t>::infinity();
        distance[id_b] = std::numeric_limits<dis_t>::infinity();

        for (siz_t j = 1; j < last; ++j) {
          distance[idx[j]] += delta * this->subtract(
            fit[idx[j + 1]], fit[idx[j - 1]], i
          );
        }

      }

      delete[] idx;
    }

    siz_t initialize (chr_t* chr, fit_t* fit, siz_t) override {
      this->evo_t::initialize(chr, fit, this->popsize());
      this->select(chr, fit, 0, this->popsize());

      return this->popsize();
    }

    siz_t evolve (chr_t* chr, fit_t* fit, siz_t) override {
      return this->evo_t::evolve(chr, fit, this->children());
    }

    siz_t select (chr_t* chr, fit_t* fit, siz_t, siz_t all) override {
      siz_t* ranked = new siz_t[all];
      siz_t* ends = new siz_t[all];

      siz_t const fronts = this->nd_sorting(fit, this->fro_, ranked, ends, all, this->popsize());
      siz_t const found = ends[fronts - 1];

      this->fronts_ = fronts;

      for (siz_t i = 0, start = 0; i < fronts; start = ends[i], ++i) {
        this->cd_sorting(fit, this->dis_, ranked + start, ends[i] - start);
      }

      if (found > this->popsize()) {
        siz_t const conti = (fronts < 2) ? 0 : ends[fronts - 2];
        siz_t const rem = this->popsize() - conti;

        util::iterator::multipartition(this->make_crowding_comparator(),
          ranked + conti, ranked + found, rem, ranked + conti
        );
      }

      util::iterator::multiorder(
        ranked, ranked + all, this->popsize(), chr, fit, this->dis_, this->fro_
      );

      delete[] ends;
      delete[] ranked;
      return this->popsize();
    }

    void on_before_user_change (void) override {
      throw std::runtime_error("Cannot update population on NSGA.");
    }

   public:

    nsga (siz_t popsize, siz_t children, siz_t dimensions, siz_t seed)
    : evo_t{ dimensions, popsize + children, seed },
      popsize_{ popsize }, children_{ children } { this->alloc(false); }

    nsga (siz_t popsize, siz_t dimensions, siz_t seed)
    : nsga{ popsize, popsize, dimensions, seed } {}

    nsga (nsga const& ot) : __EVO_BASE{ ot } {
      this->copy_from(ot, false);
    }

    nsga (nsga&& ot) : __EVO_BASE{ std::move(ot) } {
      this->move_from(ot, false);
    }

    nsga& operator = (nsga const& ot) { return this->copy_from(ot, true); }
    nsga& operator = (nsga&& ot) { return this->move_from(ot, true); }

    evo_t* copy (void) const override { return new nsga(*this); }

    siz_t popsize (void) const { return this->popsize_; }
    siz_t children (void) const { return this->children_; }
    siz_t fronts (void) const { return this->fronts_; }

    siz_t front_at (siz_t pos) const { return this->fro_[pos]; }
    dis_t distance_at (siz_t pos) const { return this->dis_[pos]; }

    bool crowding_compare (dis_t da, dis_t db) const { return da > db; }

    bool crowded_compare (siz_t fa, dis_t da, siz_t fb, dis_t db) const {
      return fa < fb or (fa == fb and this->crowding_compare(da, db));
    }

    bool crowded_compare (siz_t const& a, siz_t const& b) const {
      return this->crowded_compare(
        this->front_at(a), this->distance_at(a),
        this->front_at(b), this->distance_at(b)
      );
    }

    bool crowded_compare (
      siz_t const* fro, dis_t const* dis, siz_t const& a, siz_t const& b
    ) const {
      return this->crowded_compare(fro[a], dis[a], fro[b], dis[b]);
    }

    index_comparator make_crowding_comparator (dis_t const* dis) const {
      return [ this, dis ] (siz_t const& a, siz_t const& b) {
        return this->crowding_compare(dis[a], dis[b]);
      };
    }

    index_comparator make_crowding_comparator (void) const {
      return [ this ] (siz_t const& a, siz_t const& b) {
        return this->crowding_compare(
          this->distance_at(a), this->distance_at(b)
        );
      };
    }

    index_comparator make_crowded_comparator (
      siz_t const* fro, dis_t const* dis
    ) const {
      return [ this, fro, dis ] (siz_t const& a, siz_t const& b) {
        return this->crowded_compare(fro, dis, a, b);
      };
    }

    index_comparator make_crowded_comparator (void) const {
      return [ this ] (siz_t const& a, siz_t const& b) {
        return this->crowded_compare(a, b);
      };
    }

    expand_all_const_iterators((void), const_fro_iterator, this->fro_, this->fro_ + this->size(), fro, false);
    expand_all_const_iterators((void), const_dis_iterator, this->dis_, this->dis_ + this->size(), dis, false);
  };

};
