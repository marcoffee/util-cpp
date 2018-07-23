#pragma once

#define __EVO_TMPL_ARGS \
typename CHR, typename FIT, typename RND

#define __EVO_TMPL_HEAD_ARGS \
typename CHR, typename FIT = double, typename RND = std::mt19937

#define __EVO_NAMESPACE \
  util::evolution

#define __EVO_TMPL \
  template <__EVO_TMPL_ARGS>

#define __EVO_TMPL_HEAD \
  template <__EVO_TMPL_HEAD_ARGS>

#define __EVO_CLASS(cls) \
  cls<CHR, FIT, RND>

#define __EVO_BASE \
  __EVO_CLASS(base)

#define __EVO_COPY_TYPES(FROM) \
  using chr_t = FROM::chr_t; \
  using fit_t = FROM::fit_t; \
  using siz_t = FROM::siz_t; \
  using dis_t = FROM::dis_t; \
  using rnd_t = FROM::rnd_t; \
  using sed_t = FROM::sed_t; \
  using chr_v = FROM::chr_v; \
  using fit_v = FROM::fit_v; \
  using siz_v = FROM::siz_v; \
  using const_chr_iterator = FROM::const_chr_iterator; \
  using const_fit_iterator = FROM::const_fit_iterator; \
  \
  using step_event = FROM::step_event

#define __EVO_USING_TYPES \
  using evo_t = __EVO_BASE; \
  using chr_t = CHR; \
  using fit_t = FIT; \
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

#define __EVO_COPY_GENERATORS_FUNCTIONS(FROM) \
  using mutator = FROM::mutator

#define __EVO_USING_GENERATORS_FUNCTIONS \
  using mutator = std::function<chr_v(evo_t&, chr_t const& chr)>

#define __EVO_COPY_GENERATORS(FROM) \
  using generators = FROM::generators; \
  __EVO_COPY_GENERATORS_FUNCTIONS(FROM)

#define __EVO_USING_GENERATORS \
  using generators = __EVO_CLASS(util::evolution::generators); \
  __EVO_USING_GENERATORS_FUNCTIONS

#define __EVO_COPY_FUNCTIONS(FROM) \
  using creator    = FROM::creator; \
  using generator  = FROM::generator; \
  using evaluator  = FROM::evaluator; \
  using comparator = FROM::comparator; \
  using subtractor = FROM::subtractor; \
  \
  using simple_creator    = FROM::simple_creator; \
  using simple_generator  = FROM::simple_generator; \
  using simple_evaluator  = FROM::simple_evaluator; \
  using simple_comparator = FROM::simple_comparator; \
  using simple_subtractor = FROM::simple_subtractor; \
  \
  using index_comparator  = FROM::index_comparator

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
