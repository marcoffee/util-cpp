#pragma once

#include <random>
#include <vector>
#include <memory>
#include <functional>
#include "../util_constexpr.hh"
#include "../iterator.hh"
#include "macros.hh"
#include "generators.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class base {
   public:
    __EVO_USING_TYPES(base);
    __EVO_USING_FUNCTIONS;

    static siz_t tournament (
      evo_t& evo, siz_t size, siz_t t_size, index_comparator const& cmp
    ) {
      using dist_t = std::uniform_int_distribution<siz_t>;
      using dist_p = dist_t::param_type;

      dist_t dist;
      siz_t *idx = new siz_t[size];

      std::iota(idx, idx + size, 0);

      for (siz_t i = 0; i < t_size; ++i) {
        std::swap(idx[i], idx[dist(evo.random(), dist_p(i, size - 1))]);
      }

      siz_t const choice = *std::min_element(idx, idx + t_size, cmp);

      delete[] idx;
      return choice;
    }

    static siz_t roulette (
      evo_t& evo, siz_t size, index_comparator const& cmp
    ) {
      std::uniform_int_distribution<siz_t> dist(0, (size * (size - 1)) / 2);
      siz_t choice = size;
      siz_t accum = dist(evo.random());
      siz_t *idx = new siz_t[size];

      std::iota(idx, idx + size, 0);
      std::stable_sort(idx, idx + size, cmp);

      for (siz_t i = 0; i < size; ++i) {
        siz_t const val = size - i;

        if (val >= accum) {
          choice = idx[i];
          break;
        }

        accum -= val;
      }

      delete[] idx;
      return choice;
    }

   private:
    rnd_t rnd_;
    siz_t dimensions_ = 0, max_size_ = 0, size_ = 0;

    chr_t* chr_ = nullptr;
    fit_t* fit_ = nullptr;

    siz_t* best_ = nullptr;
    bool* best_set_ = nullptr;

    comparator* compare_ = nullptr;
    subtractor* subtract_ = nullptr;

    creator create_ = nullptr;
    generator generate_ = nullptr;
    evaluator evaluate_ = nullptr;

    step_event before_step_ = noop<step_event>;
    step_event after_step_ = noop<step_event>;

   protected:
    virtual void alloc (bool) {
      this->chr_ = new chr_t[this->max_size()];
      this->fit_ = new fit_t[this->max_size()];

      this->best_ = new siz_t[this->dimensions()]();
      this->best_set_ = new bool[this->dimensions()]();
      this->compare_ = new comparator[this->dimensions()]();
      this->subtract_ = new subtractor[this->dimensions()]();
    }

    virtual void release (bool) {
      this->chr_ = nullptr;
      this->fit_ = nullptr;

      this->best_ = nullptr;
      this->best_set_ = nullptr;
      this->compare_ = nullptr;
      this->subtract_ = nullptr;
    }

    virtual void free (bool) {
      delete[] this->chr_;
      delete[] this->fit_;

      delete[] this->best_;
      delete[] this->best_set_;
      delete[] this->compare_;
      delete[] this->subtract_;

      this->evo_t::release(false);
    }

    void copy_meta (const base& evo, bool) {
      this->dimensions_ = evo.dimensions_;
      this->max_size_ = evo.max_size_;
      this->size_ = evo.size_;
    }

    evo_t& copy_from (const base& evo, bool) {
      if (this->max_size() != evo.max_size()) {
        this->evo_t::free(false);
        this->evo_t::copy_meta(evo, false);
        this->evo_t::alloc(false);
      } else {
        this->evo_t::copy_meta(evo, false);
      }

      std::copy(evo.chrbegin(), evo.chrend(), this->chr_);
      std::copy(evo.fitbegin(), evo.fitend(), this->fit_);

      std::copy(evo.best_, evo.best_ + evo.dimensions_, this->best_);
      std::copy(evo.best_set_, evo.best_set_ + evo.dimensions_, this->best_set_);
      std::copy(evo.compare_, evo.compare_ + evo.dimensions_, this->compare_);
      std::copy(evo.subtract_, evo.subtract_ + evo.dimensions_, this->subtract_);

      this->rnd_ = evo.rnd_;

      this->create_ = evo.create_;
      this->generate_ = evo.generate_;
      this->evaluate_ = evo.evaluate_;

      return *this;
    }

    evo_t& move_from (base& evo, bool) {
      this->evo_t::free(false);
      this->evo_t::copy_meta(evo, false);

      this->chr_ = std::move(evo.chr_);
      this->fit_ = std::move(evo.fit_);

      this->best_ = std::move(evo.best_);
      this->best_set_ = std::move(evo.best_set_);
      this->compare_ = std::move(evo.compare_);
      this->subtract_ = std::move(evo.subtract_);

      this->rnd_ = std::move(evo.rnd_);

      this->create_ = std::move(evo.create_);
      this->generate_ = std::move(evo.generate_);
      this->evaluate_ = std::move(evo.evaluate_);

      evo.release(false);
      return *this;
    }

    siz_t find_best (fit_t const* fit, siz_t size, siz_t dim = 0) {
      return this->compare_[dim] ? *std::min_element(
        util::iterator::range<siz_t>(0), util::iterator::range<siz_t>(size),
        this->make_index_comparator(fit, dim)
      ) : 0;
    }

    siz_t find_worst (fit_t const* fit, siz_t size, siz_t dim = 0) {
      return this->compare_[dim] ? *std::max_element(
        util::iterator::range<siz_t>(0), util::iterator::range<siz_t>(size),
        this->make_index_comparator(fit, dim)
      ) : 0;
    }

    siz_t update_best (siz_t dim = 0) {
      if (!this->best_set_[dim]) {
        this->best_[dim] = this->find_best(this->fit_, this->size(), dim);
        this->best_set_[dim] = true;
      }

      return this->best_[dim];
    }

    void reset_best (void) {
      std::fill(this->best_set_, this->best_set_ + this->dimensions(), false);
    }

    virtual siz_t initialize (chr_t* chr, fit_t* fit, siz_t space) {
      for (siz_t i = 0; i < space; ++i) {
        chr[i] = this->create();
        fit[i] = this->evaluate(chr[i]);
      }

      return space;
    }

    virtual siz_t evolve (chr_t* chr, fit_t* fit, siz_t space) {
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

    virtual siz_t evolve (chr_t* chr, fit_t* fit, siz_t old, siz_t all) {
      return old + this->evolve(chr + old, fit + old, all - old);
    }

    virtual siz_t select (chr_t* chr, fit_t* fit, siz_t old, siz_t all) = 0;

    virtual void on_before_start (void) {}

    virtual void on_before_user_change (void) {}
    virtual void on_after_user_change (void) { this->reset_best(); }

    siz_t evolve (siz_t old, siz_t all) {
      return this->evolve(this->chr_, this->fit_, old, all);
    }

    siz_t select (siz_t old, siz_t all) {
      return this->select(this->chr_, this->fit_, old, all);
    }

   public:

    base (siz_t dimensions, siz_t max_size, sed_t seed)
    : rnd_{ seed }, dimensions_{ dimensions }, max_size_{ max_size } {
      this->evo_t::alloc(false);
    }

    base (siz_t max_size, sed_t seed) : base{ 1, max_size, seed } {}

    base (base const& evo) { this->copy_from(evo, true); }
    base (base&& evo) { this->move_from(evo, true); };

    base& operator = (base const& evo) { return this->copy_from(evo, true); }
    base& operator = (base&& evo) { return this->move_from(evo, true); }

    virtual ~base (void) { this->destroy(); };
    void destroy (void) { this->free(true); }

    virtual evo_t* copy (void) const = 0;

    void set_comparator (comparator const& cmp, siz_t dim = 0) {
      this->compare_[dim] = cmp;
    }

    void set_subtractor (subtractor const& sub, siz_t dim = 0) {
      this->subtract_[dim] = sub;
    }

    void set_creator (creator const& crt) { this->create_ = crt; }
    void set_evaluator (evaluator const& evl) { this->evaluate_ = evl; }
    void set_generator (generator const& gen) { this->generate_ = gen; }

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

    void set_evaluator (simple_evaluator const& evl) {
      this->set_evaluator([ evl ] (evo_t&, chr_t const& chr) {
        return evl(chr);
      });
    }

    void on_before_step (step_event const& func) { this->before_step_ = func; }
    void on_after_step (step_event const& func) { this->after_step_ = func; }

    simple_comparator make_simple_comparator (siz_t dim = 0) {
      return [ this, dim ] (fit_t const& a, fit_t const& b) -> bool {
        return this->compare(a, b, dim);
      };
    }

    index_comparator make_index_comparator (fit_t const* fit, siz_t dim = 0) {
      return [ this, fit, dim ] (siz_t const& a, siz_t const& b) -> bool {
        return this->compare(fit[a], fit[b], dim);
      };
    }

    index_comparator make_index_comparator (siz_t dim = 0) {
      return [ this, dim ] (siz_t const& a, siz_t const& b) -> bool {
        return this->compare(this->fit_at(a), this->fit_at(b), dim);
      };
    }

    bool compare (fit_t const& fa, fit_t const& fb, siz_t dim = 0) {
      return this->compare_[dim](*this, fa, fb);
    }

    double subtract (fit_t const& fa, fit_t const& fb, siz_t dim = 0) {
      return this->subtract_[dim](*this, fa, fb);
    }

    chr_t create (void) { return this->create_(*this); }
    chr_v generate (void) { return this->generate_(*this); }
    fit_t evaluate (chr_t& chr) { return this->evaluate_(*this, chr); }

    void populate (siz_t size) {
      this->on_before_start();

      siz_t const old = this->size();
      siz_t const pop = std::min(size, this->max_size());

      this->size_ = this->initialize(this->chr_, this->fit_, pop);

      if (old > this->size()) {
        std::destroy(this->chr_ + this->size(), this->chr_ + old);
      }

      this->reset_best();
    }

    void sort (chr_t* chr, fit_t* fit, siz_t size, siz_t dim = 0) {
      util::iterator::multisort(
        this->make_simple_comparator(dim), fit, fit + size, chr, fit
      );
    }

    void partition_best (chr_t* chr, fit_t* fit, siz_t size, siz_t nth, siz_t dim = 0) {
      if (nth > 0 and nth < size) {
        util::iterator::multipartition(
          this->make_simple_comparator(dim), fit, fit + size, nth, chr, fit
        );
      }
    }

    void partition_worst (chr_t* chr, fit_t* fit, siz_t size, siz_t nth, siz_t dim = 0) {
      if (nth > 0 and nth < size) {
        util::iterator::multipartition(
          std::not_fn(this->make_simple_comparator(dim)),
          std::make_reverse_iterator(fit + size),
          std::make_reverse_iterator(fit), nth,
          std::make_reverse_iterator(chr + size),
          std::make_reverse_iterator(fit + size)
        );
      }
    }

    siz_t tournament (siz_t t_size, index_comparator const& cmp) {
      return evo_t::tournament(*this, this->size(), t_size, cmp);
    }

    siz_t roulette (index_comparator const& cmp) {
      return evo_t::roulette(*this, this->size(), cmp);
    }

    siz_t tournament (siz_t t_size, siz_t dim = 0) {
      return this->tournament(t_size, this->make_index_comparator(dim));
    }

    siz_t roulette (siz_t dim = 0) {
      return this->roulette(this->make_index_comparator(dim));
    }

    void step (void) {
      this->before_step_(*this);

      siz_t const old = this->size();
      siz_t const all = this->max_size();

      this->size_ = this->select(old, this->evolve(old, all));
      this->reset_best();

      if (this->size() < old) {
        std::destroy(this->chr_ + this->size(), this->chr_ + old);
        std::destroy(this->fit_ + this->size(), this->fit_ + old);
      }

      this->after_step_(*this);
    }

    inline void swap (siz_t pos, chr_t& chr, fit_t& fit) {
      this->on_before_user_change();
      std::swap(this->chr_[pos], chr);
      std::swap(this->fit_[pos], fit);
      this->on_after_user_change();
    }

    inline void swap (evo_t& evo, siz_t pos, siz_t evo_p) {
      evo.on_before_user_change();
      this->swap(pos, evo.chr_[evo_p], evo.fit_[evo_p]);
      evo.on_after_user_change();
    }

    inline void remove (siz_t pos, chr_t& chr = defref<chr_t>, fit_t& fit = defref<fit_t>) {
      this->on_before_user_change();

      siz_t const last = this->size() - 1;

      std::swap(this->chr_[pos], this->chr_[last]);
      std::swap(this->fit_[pos], this->fit_[last]);

      chr = std::move(this->chr_[last]);
      fit = std::move(this->fit_[last]);

      this->size_ -= 1;

      std::destroy_at(this->chr_ + last);
      std::destroy_at(this->fit_ + last);

      this->on_after_user_change();
    }

    inline siz_t add (chr_t&& chr, fit_t&& fit) {
      this->on_before_user_change();

      siz_t const last = this->size();

      this->chr_[last] = std::move(chr);
      this->fit_[last] = std::move(fit);

      this->size_ += 1;
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
      this->chr_[pos] = std::move(chr);
      this->fit_[pos] = std::move(fit);
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
      std::move(chrs.begin(), chrs.end(), this->chr_ + pos);
      std::move(fits.begin(), fits.end(), this->fit_ + pos);
      this->on_after_user_change();
    }

    inline void set (siz_t pos, chr_v const& chrs, fit_v const& fits) {
      this->set(pos, chr_v(chrs), fit_v(fits));
    }

    void set (siz_t pos, chr_v&& chrs) {
      fit_v fits;
      fits.reserve(chrs.size());

      for (chr_t& chr : chrs) {
        fits.emplace_back(this->evaluate(chr));
      }

      this->set(pos, std::move(chrs), std::move(fits));
    }

    inline void set (siz_t pos, chr_v const& chrs) {
      this->set(pos, chr_v(chrs));
    }

    virtual siz_t best (siz_t dim = 0) { return this->update_best(dim); }

    inline rnd_t& random (void) { return this->rnd_; }
    inline rnd_t const& random (void) const { return this->rnd_; }

    inline chr_t const& chr_at (siz_t pos) const { return this->chr_[pos]; }
    inline fit_t const& fit_at (siz_t pos) const { return this->fit_[pos]; }

    inline chr_t const& best_chr (siz_t dim = 0) {
      return this->chr_at(this->best(dim));
    }

    inline fit_t const& best_fit (siz_t dim = 0) {
      return this->fit_at(this->best(dim));
    }

    inline siz_t size (void) const { return this->size_; }
    inline siz_t max_size (void) const { return this->max_size_; }
    inline siz_t dimensions (void) const { return this->dimensions_; }

    expand_all_const_iterators((void), const_chr_iterator, this->chr_, this->chr_ + this->size(), chr, false);
    expand_all_const_iterators((void), const_fit_iterator, this->fit_, this->fit_ + this->size(), fit, false);
  };

};
