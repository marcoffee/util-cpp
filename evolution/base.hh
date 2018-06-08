#pragma once

#include <random>

#define __EVO_TMPL_HEAD \
template <typename T, typename F = double, typename R = std::mt19937>

#define __EVO_CLASS(cls) \
cls<T, F, R>

#define __EVO_TMPL(cls, type) \
template <typename T, typename F, typename R>\
type __EVO_CLASS(cls)

#define __EVO_USING \
  using evo_t = base<T, F, R>;\
  using chr_t = T;\
  using fit_t = F;\
  using rnd_t = R

namespace util::evolution {

  __EVO_TMPL_HEAD
  class base {
  public:
    __EVO_USING;

    static constexpr bool has_mutation = false;
    static constexpr bool has_crossover = false;
    static constexpr bool has_population = false;
    static constexpr bool has_multi_objective = false;

    using creator    = std::function<chr_t(rnd_t&)>;
    using mutator    = std::function<chr_t(chr_t const&, rnd_t&)>;
    using crossover  = std::function<chr_t(chr_t const&, chr_t const&, rnd_t&)>;
    using evaluator  = std::function<fit_t(chr_t&)>;
    using comparator = std::function<bool(fit_t const&, fit_t const&)>;
    using sorter     = std::function<bool(uintmax_t, uintmax_t)>;

    using const_iterator = chr_t const*;
    using const_fit_iterator = fit_t const*;

    using const_reverse_iterator = std::reverse_iterator<chr_t const*>;
    using const_reverse_fit_iterator = std::reverse_iterator<fit_t const*>;

  protected:
    rnd_t _rnd;

    virtual void alloc (bool = false) {}
    virtual void free (bool = false) { this->evo_t::release(); }
    virtual void release (bool = false) {}

    constexpr void copy_meta (const base& evo, bool = false) {}

    constexpr base& copy_from (const base& evo, bool = false) {
      this->evo_t::copy_meta(evo);
      this->evo_t::free();
      this->_rnd = evo._rnd;
      return *this;
    }

    constexpr base& move_from (base& evo, bool = false) {
      this->evo_t::copy_meta(evo);
      this->_rnd = std::move(evo._rnd);
      evo.evo_t::release();
      return *this;
    }

  public:
    constexpr base (uintmax_t seed = 0) : _rnd(seed) {}

    constexpr base (base const& evo) { this->copy_from(evo); }
    constexpr base (base&& evo) { this->move_from(evo); };

    constexpr base& operator = (base const& evo) { return this->copy_from(evo); }
    constexpr base& operator = (base&& evo) { return this->move_from(evo); }

    virtual ~base (void) = default;

    virtual inline void step (void) {}

    virtual inline chr_t const chr_at (uintmax_t pos) const { return *(this->cbegin() + pos); }
    virtual inline fit_t const fit_at (uintmax_t pos) const { return *(this->cfitbegin() + pos); }

    virtual inline chr_t const best_chr (void) const { return *this->cbegin(); };
    virtual inline fit_t const best_fit (void) const { return *this->cfitbegin(); };

    virtual inline uintmax_t size (void) const { return 0; }

    virtual inline const_iterator cbegin (void) const { return nullptr; }
    virtual inline const_iterator cend (void) const { return this->cbegin() + this->size(); }

    virtual inline const_fit_iterator cfitbegin (void) const { return nullptr; }
    virtual inline const_fit_iterator cfitend (void) const { return this->cfitbegin() + this->size(); }

    virtual inline const_iterator begin (void) const { return this->cbegin(); }
    virtual inline const_iterator end (void) const { return this->cend(); }

    virtual inline const_fit_iterator fitbegin (void) const { return this->cfitbegin(); }
    virtual inline const_fit_iterator fitend (void) const { return this->cfitend(); }

    virtual inline const_reverse_iterator crbegin (void) const { return const_reverse_iterator(this->cend()); }
    virtual inline const_reverse_iterator crend (void) const { return const_reverse_iterator(this->cbegin()); }

    virtual inline const_reverse_fit_iterator crfitbegin (void) const { return const_reverse_fit_iterator(this->cfitend()); }
    virtual inline const_reverse_fit_iterator crfitend (void) const { return const_reverse_fit_iterator(this->cfitbegin()); }

    virtual inline const_reverse_iterator rbegin (void) const { return this->crbegin(); }
    virtual inline const_reverse_iterator rend (void) const { return this->crend(); }

    virtual inline const_reverse_fit_iterator rfitbegin (void) const { return this->crfitbegin(); }
    virtual inline const_reverse_fit_iterator rfitend (void) const { return this->crfitend(); }
  };

};
