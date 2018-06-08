#pragma once

#include <iterator>
#include <algorithm>
#include <functional>
#include "base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class genetic : public base<T, F, R> {
  public:
    __EVO_USING;

    using typename evo_t::creator;
    using typename evo_t::mutator;
    using typename evo_t::crossover;
    using typename evo_t::evaluator;
    using typename evo_t::comparator;
    using typename evo_t::sorter;

    using typename evo_t::const_iterator;
    using typename evo_t::const_fit_iterator;

    static constexpr bool has_mutation = true;
    static constexpr bool has_crossover = true;
    static constexpr bool has_population = true;

  private:

    mutator _mutate;
    crossover _cross;
    evaluator _evaluate;
    comparator _compare;
    sorter _argsort;

    uintmax_t _p_size = 0, _t_size = 0;
    double _m_prob = 0;

    uintmax_t _elitism = 1;
    bool _always_cross = false;

    chr_t* _chr = nullptr;
    fit_t* _fit = nullptr;
    chr_t* _tmp_chr = nullptr;
    fit_t* _tmp_fit = nullptr;
    uintmax_t* _tmp_idx = nullptr;

    void create_argsort (void) {
      this->_argsort = [&] (uintmax_t a, uintmax_t b) -> bool {
        return this->_compare(this->_tmp_fit[a], this->_tmp_fit[b]);
      };
    }

    void transfer (void);
    uintmax_t select (void);

    constexpr void alloc (bool rec = true) override;
    constexpr void release (bool rec = true) override;
    constexpr void free (bool rec = true) override;
    constexpr void copy_meta (genetic const& evo, bool rec = true);
    constexpr genetic& copy_from (genetic const& evo, bool rec = true);
    constexpr genetic& move_from (genetic& evo, bool rec = true);

  public:

    constexpr genetic (void) { this->create_argsort(); }

    constexpr genetic (
      uintmax_t seed,
      creator create, mutator mutate, crossover cross,
      evaluator evaluate, comparator compare,
      uintmax_t p_size, uintmax_t t_size, double m_prob
    );

    constexpr genetic (genetic const& evo) {
      this->create_argsort();
      this->copy_from(evo);
    }

    constexpr genetic (genetic&& evo) {
      this->create_argsort();
      this->move_from(evo);
    }

    constexpr genetic& operator = (genetic const& evo) { return this->copy_from(evo); }
    constexpr genetic& operator = (genetic&& evo) { return this->move_from(evo); }

    ~genetic (void);

    void step (void) override;

    template <typename p_it, typename c_it, typename f_it>
    void swap (p_it p_s, c_it c_s, f_it f_s, uintmax_t amount, bool sort = true);

    constexpr uintmax_t const p_size (void) const { return this->_p_size; }
    constexpr uintmax_t const t_size (void) const { return this->_t_size; }
    constexpr double const m_prob (void) const { return this->_m_prob; }

    constexpr void t_size (uintmax_t value) { this->_t_size = std::min(this->p_size(), value); }
    constexpr void m_prob (double value) { this->_m_prob = value; }

    constexpr uintmax_t const elitism (void) const { return this->_elitism; }
    constexpr bool const always_cross (void) const { return this->_always_cross; }

    constexpr uintmax_t size (void) const override { return this->p_size(); }

    constexpr void elitism (uintmax_t value) { this->_elitism = std::min(this->p_size(), value); }
    constexpr void always_cross (bool value) { this->_always_cross = value; }

    constexpr const_iterator cbegin (void) const override { return this->_chr; }
    constexpr const_fit_iterator cfitbegin (void) const override { return this->_fit; }
  };

  __EVO_TMPL(genetic, constexpr void)::alloc (bool rec) {
    if (rec) {
      genetic::evo_t::alloc(true);
    }

    this->_chr = new chr_t[this->p_size()];
    this->_fit = new fit_t[this->p_size()];
    this->_tmp_chr = new chr_t[this->p_size()];
    this->_tmp_fit = new fit_t[this->p_size()];
    this->_tmp_idx = new uintmax_t[this->p_size()];
  }

  __EVO_TMPL(genetic, constexpr void)::release (bool rec) {
    if (rec) {
      genetic::evo_t::release(true);
    }

    this->_tmp_chr = this->_chr = nullptr;
    this->_tmp_fit = this->_fit = nullptr;
    this->_tmp_idx = nullptr;
  }

  __EVO_TMPL(genetic, constexpr void)::free (bool rec) {
    if (rec) {
      genetic::evo_t::free(true);
    }

    if (this->_chr) {
      delete[] this->_chr;
      delete[] this->_fit;
      delete[] this->_tmp_chr;
      delete[] this->_tmp_fit;
      delete[] this->_tmp_idx;

      this->release(false);
    }
  }

  __EVO_TMPL(genetic, constexpr void)::copy_meta (genetic const& evo, bool rec) {
    if (rec) {
      genetic::evo_t::copy_meta(evo, true);
    }

    this->_p_size = evo._p_size;
    this->_t_size = evo._t_size;
    this->_m_prob = evo._m_prob;
    this->_elitism = evo._elitism;
    this->_always_cross = evo._always_cross;
  }

  __EVO_TMPL(genetic, constexpr __EVO_CLASS(genetic)&)::copy_from (
    genetic const& evo, bool rec
  ) {
    if (rec) {
      genetic::evo_t::copy_from(evo, true);
    }

    bool const diff = this->_p_size != evo._p_size;

    if (diff) {
      this->free(false);
    }

    this->copy_meta(evo, false);

    if (diff) {
      this->alloc(false);
    }

    this->_mutate = evo._mutate;
    this->_cross = evo._cross;
    this->_evaluate = evo._evaluate;
    this->_compare = evo._compare;

    std::copy_n(evo._chr, this->p_size(), this->_chr);
    std::copy_n(evo._fit, this->p_size(), this->_fit);

    return *this;
  }

  __EVO_TMPL(genetic, constexpr __EVO_CLASS(genetic)&)::move_from (
    genetic& evo, bool rec
  ) {
    if (rec) {
      genetic::evo_t::move_from(evo, true);
    }

    this->copy_meta(evo, false);
    this->free(false);

    this->_mutate = std::move(evo._mutate);
    this->_cross = std::move(evo._cross);
    this->_evaluate = std::move(evo._evaluate);
    this->_compare = std::move(evo._compare);

    this->_chr = evo._chr;
    this->_fit = evo._fit;
    this->_tmp_chr = evo._tmp_chr;
    this->_tmp_fit = evo._tmp_fit;
    this->_tmp_idx = evo._tmp_idx;

    evo.release();
    return *this;
  }

  __EVO_TMPL(genetic, void)::transfer (void) {
    std::iota(this->_tmp_idx, this->_tmp_idx + this->p_size(), 0);
    std::sort(this->_tmp_idx, this->_tmp_idx + this->p_size(), this->_argsort);

    for (uintmax_t i = 0; i < this->p_size(); ++i) {
      uintmax_t a = this->_tmp_idx[i];
      this->_chr[i] = std::move(this->_tmp_chr[a]);
      this->_fit[i] = std::move(this->_tmp_fit[a]);
    }
  }

  __EVO_TMPL(genetic, uintmax_t)::select (void) {
    uintmax_t const t_size = this->t_size();

    std::iota(this->_tmp_idx, this->_tmp_idx + this->p_size(), 0);
    std::shuffle(this->_tmp_idx, this->_tmp_idx + this->p_size(), this->_rnd);

    uintmax_t choice = 0;

    for (uintmax_t i = 1; i < t_size; ++i) {
      if (this->_tmp_idx[i] < this->_tmp_idx[choice]) {
        choice = i;
      }
    }

    return this->_tmp_idx[choice];
  }

  __EVO_TMPL(genetic, constexpr)::genetic (
    uintmax_t seed,
    creator create, mutator mutate, crossover cross,
    evaluator evaluate, comparator compare,
    uintmax_t p_size, uintmax_t t_size, double m_prob
  ) : base<T, F, R>(seed),
      _mutate(mutate), _cross(cross), _evaluate(evaluate), _compare(compare),
      _p_size(std::max(UINTMAX_C(1), p_size)),
      _t_size(std::min(p_size, t_size)),
      _m_prob(m_prob) {

    this->create_argsort();
    this->alloc();

    for (uintmax_t i = 0; i < this->p_size(); ++i) {
      chr_t indiv = create(this->_rnd);
      fit_t i_fit = this->_evaluate(indiv);
      this->_tmp_chr[i] = std::move(indiv);
      this->_tmp_fit[i] = std::move(i_fit);
    }

    this->transfer();
  }

  __EVO_TMPL(genetic, EMPTY_ARG)::~genetic (void) { this->free(); }

  __EVO_TMPL(genetic, void)::step (void) {
    std::uniform_real_distribution<double> dist;
    uintmax_t const elitism = this->elitism();
    bool const always_cross = this->always_cross();
    double const m_prob = this->m_prob();

    for (uintmax_t i = elitism; i < this->p_size(); ++i) {
      chr_t child;

      if (always_cross) {
        uintmax_t const p1 = this->select(), p2 = this->select();
        child = this->_cross(this->_chr[p1], this->_chr[p2], this->_rnd);

        if (dist(this->_rnd) <= m_prob) {
          child = this->_mutate(child, this->_rnd);
        }
      } else if (dist(this->_rnd) <= m_prob) {
        uintmax_t const p1 = this->select();
        child = this->_mutate(this->_chr[p1], this->_rnd);
      } else {
        uintmax_t const p1 = this->select(), p2 = this->select();
        child = this->_cross(this->_chr[p1], this->_chr[p2], this->_rnd);
      }

      fit_t i_fit = this->_evaluate(child);

      this->_tmp_chr[i] = std::move(child);
      this->_tmp_fit[i] = std::move(i_fit);
    }

    for (uintmax_t i = 0; i < elitism; ++i) {
      this->_tmp_chr[i] = std::move(this->_chr[i]);
      this->_tmp_fit[i] = std::move(this->_fit[i]);
    }

    this->transfer();
  }

  template <typename T, typename F, typename R>
  template <typename p_it, typename c_it, typename f_it>
  void genetic<T, F, R>::swap (
    p_it p_s, c_it c_s, f_it f_s, uintmax_t amount, bool sort
  ) {
    std::swap(this->_chr, this->_tmp_chr);
    std::swap(this->_fit, this->_tmp_fit);

    for (uintmax_t i = 0; i < amount; ++i) {
      std::swap(this->_tmp_chr[*p_s], *c_s);
      std::swap(this->_tmp_fit[*p_s], *f_s);
      ++p_s, ++c_s, ++f_s;
    }

    if (sort) {
      this->transfer();
    } else {
      std::swap(this->_chr, this->_tmp_chr);
      std::swap(this->_fit, this->_tmp_fit);
    }
  }

};
