#pragma once

#include "base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class one_lambda : public __BASE_CLASS {

  public:
    __EVO_USING;

    using typename evo_t::creator;
    using typename evo_t::mutator;
    using typename evo_t::evaluator;
    using typename evo_t::comparator;

    using typename evo_t::const_iterator;
    using typename evo_t::const_fit_iterator;

    static constexpr bool has_mutation = true;

  private:
    creator _create;
    mutator _mutate;
    evaluator _evaluate;
    comparator _compare;

    uintmax_t _lambda;

    chr_t _chr;
    fit_t _fit;

    void copy_meta (one_lambda const& evo, bool rec = true);
    one_lambda& copy_from (one_lambda const& evo, bool rec = true);
    one_lambda& move_from (one_lambda& evo, bool rec = true);

    void make_step (void) override;

    chr_t* chr_ptr (void) override { return &this->_chr; }
    fit_t* fit_ptr (void) override { return &this->_fit; }

    chr_t const* chr_ptr (void) const override { return &this->_chr; }
    fit_t const* fit_ptr (void) const override { return &this->_fit; }

  public:

    one_lambda (
      uintmax_t seed,
      creator create, mutator mutate, evaluator evaluate, comparator compare,
      uintmax_t lambda
    );

    one_lambda (one_lambda const& evo) { this->copy_from(evo); };
    one_lambda (one_lambda&& evo) { this->move_from(std::move(evo)); };

    one_lambda& operator = (one_lambda const& evo) { return this->copy_from(evo); };
    one_lambda& operator = (one_lambda&& evo) { return this->move_from(std::move(evo)); };

    ~one_lambda (void);

    evo_t* copy (void) const override { return new one_lambda(*this); }

    const uintmax_t& lambda (void) const { return this->_lambda; }
    void lambda (uintmax_t lambda) { this->_lambda = lambda; }

    uintmax_t size (void) const override { return 1; }
  };

  __EVO_TMPL(void, one_lambda)::copy_meta (one_lambda const& evo, bool rec) {
    if (rec) {
      evo_t::copy_meta(evo, true);
    }

    this->_lambda = evo._lambda;
  }

  __EVO_TMPL(__EVO_CLASS(one_lambda)&, one_lambda)::copy_from (
    one_lambda const& evo, bool rec
  ) {
    this->copy_meta(evo, false);

    if (rec) {
      evo_t::copy_from(evo, true);
    }

    this->_create = evo._create;
    this->_mutate = evo._mutate;
    this->_evaluate = evo._evaluate;
    this->_compare = evo._compare;

    this->_chr = evo._chr;
    this->_fit = evo._fit;

    return *this;
  }

  __EVO_TMPL(__EVO_CLASS(one_lambda)&, one_lambda)::move_from (
    one_lambda& evo, bool rec
  ) {
    this->copy_meta(evo, false);

    if (rec) {
      evo_t::move_from(evo, true);
    }

    this->_create = std::move(evo._create);
    this->_mutate = std::move(evo._mutate);
    this->_evaluate = std::move(evo._evaluate);
    this->_compare = std::move(evo._compare);

    this->_chr = std::move(evo._chr);
    this->_fit = std::move(evo._fit);

    return *this;
  }

  __EVO_TMPL(EMPTY_ARG, one_lambda)::one_lambda(
    uintmax_t seed,
    creator create, mutator mutate, evaluator evaluate, comparator compare,
    uintmax_t lambda
  ) : evo_t(seed),
      _create(create), _mutate(mutate), _evaluate(evaluate), _compare(compare),
      _lambda(lambda) {

    this->_chr = create(this->_rnd);
    this->_fit = this->_evaluate(this->_chr);
  }

  __EVO_TMPL(EMPTY_ARG, one_lambda)::~one_lambda (void) {}

  __EVO_TMPL(void, one_lambda)::make_step (void) {
    bool changed = false;

    chr_t old_chr(std::move(this->_chr));
    fit_t old_fit(std::move(this->_fit));

    fit_t const* best_fit = &old_fit;

    const uintmax_t lambda = this->lambda();

    for (uintmax_t i = 0; i < lambda; ++i) {
      chr_t indiv = this->_mutate(old_chr, this->_rnd);
      fit_t i_fit = this->_evaluate(indiv);

      if (this->_compare(i_fit, *best_fit)) {
        this->_chr = std::move(indiv);
        this->_fit = std::move(i_fit);
        best_fit = &this->_fit;
        changed = true;
      }
    }

    if (!changed) {
      this->_chr = std::move(old_chr);
      this->_fit = std::move(old_fit);
    }
  }

};
