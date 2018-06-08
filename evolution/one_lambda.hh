#pragma once

#include "base.hh"

namespace util::evolution {

  __EVO_TMPL_HEAD
  class one_lambda : public base<T, F, R> {

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

    chr_t _best_chr;
    fit_t _best_fit;

    void copy_meta (one_lambda const& evo, bool rec = true);
    one_lambda& copy_from (one_lambda const& evo, bool rec = true);
    one_lambda& move_from (one_lambda& evo, bool rec = true);

  public:

    one_lambda (
      uintmax_t seed,
      creator create, mutator mutate, evaluator evaluate, comparator compare,
      uintmax_t lambda
    );

    one_lambda (one_lambda const& evo) :
      evo_t(evo),
      _create(evo._create), _mutate(evo._mutate), _evaluate(evo._evaluate),
      _compare(evo._compare),
      _lambda(evo._lambda),
      _best_chr(evo._best_chr), _best_fit(evo._best_fit) {};

    ~one_lambda (void);

    void step (void) override;

    const uintmax_t& lambda (void) const { return this->_lambda; }
    void lambda (uintmax_t lambda) { this->_lambda = lambda; }

    uintmax_t size (void) const override { return 1; }

    const_iterator cbegin (void) const override { return &this->_best_chr; }
    const_fit_iterator cfitbegin (void) const override { return &this->_best_fit; }

  };

  __EVO_TMPL(one_lambda, void)::copy_meta (one_lambda const& evo, bool rec) {
    if (rec) {
      evo_t::copy_meta(evo, true);
    }

    this->_lambda = evo._lambda;
  }

  __EVO_TMPL(one_lambda, __EVO_CLASS(one_lambda)&)::copy_from (
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

    this->_best_chr = evo._best_chr;
    this->_best_fit = evo._best_fit;

    return *this;
  }

  __EVO_TMPL(one_lambda, __EVO_CLASS(one_lambda)&)::move_from (
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

    this->_best_chr = std::move(evo._best_chr);
    this->_best_fit = std::move(evo._best_fit);

    return *this;
  }

  __EVO_TMPL(one_lambda, EMPTY_ARG)::one_lambda(
    uintmax_t seed,
    creator create, mutator mutate, evaluator evaluate, comparator compare,
    uintmax_t lambda
  ) : base<T, F, R>(seed),
      _create(create), _mutate(mutate), _evaluate(evaluate), _compare(compare),
      _lambda(lambda) {

    this->_best_chr = create(this->_rnd);
    this->_best_fit = this->_evaluate(this->_best_chr);
  }

  __EVO_TMPL(one_lambda, EMPTY_ARG)::~one_lambda (void) {}

  __EVO_TMPL(one_lambda, void)::step (void) {
    bool changed = false;

    chr_t old_chr(std::move(this->_best_chr));
    fit_t old_fit(std::move(this->_best_fit));

    fit_t const* best_fit = &old_fit;

    const uintmax_t lambda = this->lambda();

    for (uintmax_t i = 0; i < lambda; ++i) {
      chr_t indiv = this->_mutate(old_chr, this->_rnd);
      fit_t i_fit = this->_evaluate(indiv);

      if (this->_compare(i_fit, *best_fit)) {
        this->_best_chr = std::move(indiv);
        this->_best_fit = std::move(i_fit);
        best_fit = &this->_best_fit;
        changed = true;
      }
    }

    if (!changed) {
      this->_best_chr = std::move(old_chr);
      this->_best_fit = std::move(old_fit);
    }
  }

};
