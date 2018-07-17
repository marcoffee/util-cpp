#pragma once

#include <random>
#include <vector>
#include <functional>
#include "../util_constexpr.hh"
#include "../iterator.hh"
#include "macros.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class base {
  public:
    __EVO_USING_TYPES;
    __EVO_USING_FUNCTIONS;

  private:
    rnd_t _rnd;
    siz_t _dimensions = 0, _max_size = 0, _size = 0;

    chr_t* _chr = nullptr;
    fit_t* _fit = nullptr;

    siz_t* _best = nullptr;
    bool* _best_set = nullptr;

    comparator* _compare = nullptr;
    subtractor* _subtract = nullptr;

    creator _create = nullptr;
    generator _generate = nullptr;
    evaluator _evaluate = nullptr;

    step_event _before_step = noop<step_event>;
    step_event _after_step = noop<step_event>;

  protected:

    virtual void alloc (bool) {
      this->_chr = new chr_t[this->max_size()];
      this->_fit = new fit_t[this->max_size()];

      this->_best = new siz_t[this->dimensions()]();
      this->_best_set = new bool[this->dimensions()]();
      this->_compare = new comparator[this->dimensions()]();
      this->_subtract = new subtractor[this->dimensions()]();
    }

    virtual void release (bool) {
      this->_chr = nullptr;
      this->_fit = nullptr;

      this->_best = nullptr;
      this->_best_set = nullptr;
      this->_compare = nullptr;
      this->_subtract = nullptr;
    }

    virtual void free (bool) {
      delete[] this->_chr;
      delete[] this->_fit;

      delete[] this->_best;
      delete[] this->_best_set;
      delete[] this->_compare;
      delete[] this->_subtract;

      this->evo_t::release(false);
    }

    void copy_meta (const base& evo, bool) {
      this->_dimensions = evo._dimensions;
      this->_max_size = evo._max_size;
      this->_size = evo._size;
    }

    evo_t& copy_from (const base& evo, bool) {
      if (this->max_size() != evo.max_size()) {
        this->evo_t::free(false);
        this->evo_t::copy_meta(evo, false);
        this->evo_t::alloc(false);
      } else {
        this->evo_t::copy_meta(evo, false);
      }

      std::copy(evo.chrbegin(), evo.chrend(), this->_chr);
      std::copy(evo.fitbegin(), evo.fitend(), this->_fit);

      std::copy(evo._best, evo._best + evo._dimensions, this->_best);
      std::copy(evo._best_set, evo._best_set + evo._dimensions, this->_best_set);
      std::copy(evo._compare, evo._compare + evo._dimensions, this->_compare);
      std::copy(evo._subtract, evo._subtract + evo._dimensions, this->_subtract);

      this->_rnd = evo._rnd;

      this->_create = evo._create;
      this->_generate = evo._generate;
      this->_evaluate = evo._evaluate;

      return *this;
    }

    evo_t& move_from (base& evo, bool) {
      this->evo_t::free(false);
      this->evo_t::copy_meta(evo, false);

      this->_chr = std::move(evo._chr);
      this->_fit = std::move(evo._fit);

      this->_best = std::move(evo._best);
      this->_best_set = std::move(evo._best_set);
      this->_compare = std::move(evo._compare);
      this->_subtract = std::move(evo._subtract);

      this->_rnd = std::move(evo._rnd);

      this->_create = std::move(evo._create);
      this->_generate = std::move(evo._generate);
      this->_evaluate = std::move(evo._evaluate);

      evo.release(false);
      return *this;
    }

    simple_comparator build_compare (siz_t dim = 0) {
      return [ this, dim ] (fit_t const& a, fit_t const& b) -> bool {
        return this->compare(a, b, dim);
      };
    }

    index_comparator build_compare (fit_t const* fit, siz_t dim = 0) {
      return [ this, fit, dim ] (siz_t a, siz_t b) -> bool {
        return this->compare(fit[a], fit[b], dim);
      };
    }

    siz_t find_best (fit_t const* fit, siz_t size, siz_t dim = 0) {
      return this->_compare[dim] ? *std::min_element(
        util::iterator::range<siz_t>(0), util::iterator::range<siz_t>(size),
        this->build_compare(fit, dim)
      ) : 0;
    }

    siz_t update_best (siz_t dim = 0) {
      if (!this->_best_set[dim]) {
        this->_best[dim] = this->find_best(
          this->_fit, this->size(), dim
        );

        this->_best_set[dim] = true;
      }

      return this->_best[dim];
    }

    void reset_best (void) {
      std::fill(this->_best_set, this->_best_set + this->dimensions(), false);
    }

    virtual siz_t initialize (chr_t* chr, fit_t* fit, siz_t space) {
      for (siz_t i = 0; i < space; ++i) {
        chr[i] = this->create();
        fit[i] = this->evaluate(chr[i]);
      }

      return space;
    }

    virtual siz_t evolve (chr_t* chr, fit_t* fit, siz_t space) {
      std::uniform_real_distribution<double> dist;

      for (siz_t i = 0; i < space; ) {
        for (chr_t& child : this->generate()) {
          chr[i] = std::move(child);
          fit[i] = this->evaluate(chr[i]);

          if (++i >= space) {
            break;
          }
        }
      }

      return space;
    }

    virtual siz_t select (chr_t* chr, fit_t* fit, siz_t old, siz_t all) = 0;

    virtual void on_before_user_change (void) {}

    virtual void on_after_user_change (void) {
      this->reset_best();
    }

    siz_t evolve (siz_t space, siz_t old) {
      return this->evolve(this->_chr + old, this->_fit + old, space);
    }

    siz_t select (siz_t old, siz_t all) {
      return this->select(this->_chr, this->_fit, old, all);
    }

  public:

    base (siz_t dimensions, siz_t max_size, sed_t seed)
      : _rnd(seed), _dimensions(dimensions), _max_size(max_size) {
      this->evo_t::alloc(false);
    }

    base (siz_t max_size, sed_t seed) : base(1, max_size, seed) {}

    base (base const& evo) { this->copy_from(evo, true); }
    base (base&& evo) { this->move_from(evo, true); };

    base& operator = (base const& evo) { return this->copy_from(evo, true); }
    base& operator = (base&& evo) { return this->move_from(evo, true); }

    virtual ~base (void) { this->destroy(); };
    void destroy (void) { this->free(true); }

    virtual evo_t* copy (void) const = 0;

    void set_comparator (comparator const& cmp, siz_t dim = 0) {
      this->_compare[dim] = cmp;
    }

    void set_subtractor (subtractor const& sub, siz_t dim = 0) {
      this->_subtract[dim] = sub;
    }

    void set_creator (creator const& crt) { this->_create = crt; }
    void set_generator (generator const& gen) { this->_generate = gen; }
    void set_evaluator (evaluator const& evl) { this->_evaluate = evl; }

    void set_comparator (simple_comparator const& cmp, siz_t dim = 0) {
      this->set_comparator([ cmp ] (evo_t&, fit_t const& f1, fit_t const& f2) {
        return cmp(f1, f2);
      }, dim);
    }

    void set_subtractor (simple_subtractor const& sub, siz_t dim = 0) {
      this->set_subtractor([ sub ] (evo_t&, fit_t const& f1, fit_t const& f2) {
        return sub(f1, f2);
      }, dim);
    }

    void set_creator (simple_creator const& crt) {
      this->set_creator([ crt ] (evo_t&) { return crt(); });
    }

    void set_generator (simple_generator const& gen) {
      this->set_generator([ gen ] (evo_t&) { return gen();});
    }

    void set_evaluator (simple_evaluator const& evl) {
      this->set_evaluator([ evl ] (evo_t&, chr_t const& chr) {
        return evl(chr);
      });
    }

    void on_before_step (step_event const& func) { this->_before_step = func; }
    void on_after_step (step_event const& func) { this->_after_step = func; }

    bool compare (fit_t const& fa, fit_t const& fb, siz_t dim = 0) {
      return this->_compare[dim](*this, fa, fb);
    }

    double subtract (fit_t const& fa, fit_t const& fb, siz_t dim = 0) {
      return this->_subtract[dim](*this, fa, fb);
    }

    chr_t create (void) { return this->_create(*this); }
    chr_v generate (void) { return this->_generate(*this); }
    fit_t evaluate (chr_t const& chr) { return this->_evaluate(*this, chr); }

    void populate (siz_t size) {
      siz_t const old = this->size();
      siz_t const pop = std::min(size, this->max_size());

      this->_size = this->initialize(this->_chr, this->_fit, pop);

      if (old > this->size()) {
        std::destroy(this->_chr + this->size(), this->_chr + old);
      }

      this->reset_best();
    }

    void sort (chr_t* chr, fit_t* fit, siz_t size, siz_t dim = 0) {
      util::iterator::multisort(
        this->build_compare(dim), fit, fit + size, chr, fit
      );
    }

    void partition_best (chr_t* chr, fit_t* fit, siz_t size, siz_t nth, siz_t dim = 0) {
      if (nth > 0 && nth < size) {
        util::iterator::multipartition(
          this->build_compare(dim), fit, fit + size, nth, chr, fit
        );
      }
    }

    void partition_worst (chr_t* chr, fit_t* fit, siz_t size, siz_t nth, siz_t dim = 0) {
      if (nth > 0 && nth < size) {
        util::iterator::multipartition(
          std::not_fn(this->build_compare(dim)),
          std::make_reverse_iterator(fit + size),
          std::make_reverse_iterator(fit), nth,
          std::make_reverse_iterator(chr + size),
          std::make_reverse_iterator(fit + size)
        );
      }
    }

    siz_t tournament (siz_t t_size, fit_t* fit, siz_t size, siz_t dim = 0) {
      siz_t *idx = new siz_t[size];

      std::iota(idx, idx + size, 0);
      std::shuffle(idx, idx + size, this->_rnd);

      siz_t const choice = *std::min_element(
        idx, idx + t_size, this->build_compare(fit, dim)
      );

      delete[] idx;
      return choice;
    }

    siz_t tournament (siz_t t_size, siz_t dim = 0) {
      return this->tournament(t_size, this->_fit, this->size(), dim);
    }

    void step (void) {
      this->_before_step(*this);

      siz_t const old = this->size();
      siz_t const space = this->max_size() - this->size();

      this->_size = this->select(old, old + this->evolve(space, old));
      this->reset_best();

      if (this->size() < old) {
        std::destroy(this->_chr + this->size(), this->_chr + old);
        std::destroy(this->_fit + this->size(), this->_fit + old);
      }

      this->_after_step(*this);
    }

    inline void swap (siz_t pos, chr_t& chr, fit_t& fit) {
      this->on_before_user_change();
      std::swap(this->_chr[pos], chr);
      std::swap(this->_fit[pos], fit);
      this->on_after_user_change();
    }

    inline void swap (evo_t& evo, siz_t pos, siz_t evo_p) {
      evo.on_before_user_change();
      this->swap(pos, evo._chr[evo_p], evo._fit[evo_p]);
      evo.on_after_user_change();
    }

    inline void remove (siz_t pos, chr_t& chr = defref<chr_t>, fit_t& fit = defref<fit_t>) {
      this->on_before_user_change();

      siz_t const last = this->size() - 1;

      std::swap(this->_chr[pos], this->_chr[last]);
      std::swap(this->_fit[pos], this->_fit[last]);

      chr = std::move(this->_chr[last]);
      fit = std::move(this->_fit[last]);

      this->_size -= 1;

      std::destroy_at(this->_chr + last);
      std::destroy_at(this->_fit + last);

      this->on_after_user_change();
    }

    inline siz_t add (chr_t&& chr, fit_t&& fit) {
      this->on_before_user_change();

      siz_t const last = this->size();

      this->_chr[last] = std::move(chr);
      this->_fit[last] = std::move(fit);

      this->_size += 1;
      this->on_after_user_change();

      return last;
    }

    inline siz_t add (chr_t const& chr, fit_t const& fit) {
      return this->add(chr_t(chr), fit_t(fit));
    }

    inline siz_t add (chr_t&& chr) {
      fit_t fit = this->evaluate(chr);
      return this->add(std::move(chr), std::move(fit));
    }

    inline siz_t add (chr_t const& chr) {
      return this->add(chr_t(chr));
    }

    inline void set (siz_t pos, chr_t&& chr, fit_t&& fit) {
      this->on_before_user_change();
      this->_chr[pos] = std::move(chr);
      this->_fit[pos] = std::move(fit);
      this->on_after_user_change();
    }

    inline void set (siz_t pos, chr_t const& chr, fit_t const& fit) {
      this->set(pos, chr_t(chr), fit_t(fit));
    }

    inline void set (siz_t pos, chr_t&& chr) {
      fit_t fit = this->evaluate(chr);
      this->set(pos, std::move(chr), std::move(fit));
    }

    inline void set (siz_t pos, chr_t chr) {
      this->set(pos, std::move(chr));
    }

    inline void set (siz_t pos, chr_v&& chrs, fit_v&& fits) {
      this->on_before_user_change();
      std::move(chrs.begin(), chrs.end(), this->_chr + pos);
      std::move(fits.begin(), fits.end(), this->_fit + pos);
      this->on_after_user_change();
    }

    inline void set (siz_t pos, chr_v const& chrs, fit_v const& fits) {
      this->set(pos, chr_v(chrs), fit_v(fits));
    }

    void set (siz_t pos, chr_v&& chrs) {
      fit_v fits;
      fits.reserve(chrs.size());

      for (chr_t const& chr : chrs) {
        fits.emplace_back(this->evaluate(chr));
      }

      this->set(pos, std::move(chrs), std::move(fits));
    }

    inline void set (siz_t pos, chr_v const& chrs) {
      this->set(pos, chr_v(chrs));
    }

    virtual siz_t best (siz_t dim = 0) { return this->update_best(dim); }

    inline rnd_t& random (void) { return this->_rnd; }
    inline rnd_t const& random (void) const { return this->_rnd; }

    inline chr_t const& chr_at (siz_t pos) const { return this->_chr[pos]; }
    inline fit_t const& fit_at (siz_t pos) const { return this->_fit[pos]; }

    inline chr_t const& best_chr (siz_t dim = 0) {
      return this->chr_at(this->best(dim));
    }

    inline fit_t const& best_fit (siz_t dim = 0) {
      return this->fit_at(this->best(dim));
    }

    inline siz_t size (void) const { return this->_size; }
    inline siz_t max_size (void) const { return this->_max_size; }
    inline siz_t dimensions (void) const { return this->_dimensions; }

    expand_all_const_iterators((void), const_chr_iterator, this->_chr, this->_chr + this->size(), chr, false)
    expand_all_const_iterators((void), const_fit_iterator, this->_fit, this->_fit + this->size(), fit, false)
  };

};
