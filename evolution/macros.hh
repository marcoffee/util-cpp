#pragma once

#define __EVO_TMPL_ARGS \
typename T, typename F, typename RND

#define __EVO_TMPL_HEAD_ARGS \
typename T, typename F = double, typename RND = std::mt19937

#define __EVO_TMPL \
template <__EVO_TMPL_ARGS>

#define __EVO_TMPL_HEAD \
template <__EVO_TMPL_HEAD_ARGS>

#define __EVO_CLASS(cls) \
cls<T, F, RND>

#define __BASE_CLASS \
__EVO_CLASS(base)

#define __EVO_USING_TYPES \
  using evo_t = __BASE_CLASS; \
  using chr_t = T; \
  using fit_t = F; \
  using siz_t = uintmax_t; \
  using dis_t = double; \
  using rnd_t = RND; \
  using sed_t = typename rnd_t::result_type; \
  using chr_v = std::vector<chr_t>; \
  using fit_v = std::vector<fit_t>; \
  using siz_v = std::vector<siz_t>; \
  using const_chr_iterator = chr_t const*; \
  using const_fit_iterator = fit_t const*; \
  \
  using step_event = std::function<void(evo_t&)>

#define __EVO_USING_GENERATOR_FUNCTIONS \
  using single_mutator = std::function<chr_v(evo_t&, chr_t const& chr)>; \
  using crossover      = std::function<chr_v(evo_t&)>; \
  using mutator        = std::function<chr_v(evo_t&)>;

#define __EVO_USING_GENERATOR \
  using generators     = __EVO_CLASS(util::evolution::generators); \
  __EVO_USING_GENERATOR_FUNCTIONS

#define __EVO_USING_FUNCTIONS \
  using creator    = std::function<chr_t(evo_t&)>; \
  using generator  = std::function<chr_v(evo_t&)>; \
  using evaluator  = std::function<fit_t(evo_t&, chr_t const&)>; \
  using comparator = std::function<bool(evo_t&, fit_t const&, fit_t const&)>; \
  using subtractor = std::function<dis_t(evo_t&, fit_t const&, fit_t const&)>; \
  \
  using simple_creator    = std::function<chr_t()>; \
  using simple_generator  = std::function<chr_v()>; \
  using simple_evaluator  = std::function<fit_t(chr_t const&)>; \
  using simple_comparator = std::function<bool(fit_t const&, fit_t const&)>; \
  using simple_subtractor = function_ret_rebind<simple_comparator, double>; \
  \
  using index_comparator  = std::function<bool(siz_t const&, siz_t const&)>
