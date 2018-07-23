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

#define __EVO_CLASS_HEAD(name) \
  class name : __EVO_BASE

#define __EVO_COPY_TYPES(name, from) \
  using gen_t = from::gen_t; \
  using evo_t = from::evo_t; \
  using chr_t = from::chr_t; \
  using fit_t = from::fit_t; \
  using siz_t = from::siz_t; \
  using dis_t = from::dis_t; \
  using rnd_t = from::rnd_t; \
  using sed_t = from::sed_t; \
  using chr_v = from::chr_v; \
  using fit_v = from::fit_v; \
  using siz_v = from::siz_v; \
  using generators = from::generators; \
  using const_chr_iterator = from::const_chr_iterator; \
  using const_fit_iterator = from::const_fit_iterator; \
  \
  using step_event = from::step_event; \
  constexpr static char const* class_name = #name

#define __EVO_USING_TYPES(name) \
  using gen_t = name; \
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
  using generators = util::evolution::base_generators<gen_t>; \
  using const_chr_iterator = chr_t const*; \
  using const_fit_iterator = fit_t const*; \
  \
  using step_event = std::function<void(evo_t&)>; \
  constexpr static char const* class_name = #name

#define __EVO_USING_FUNCTIONS \
  using index_comparator = std::function<bool(siz_t const&, siz_t const&)>; \
  \
  using generator = std::function<chr_v(evo_t&)>; \
  using mutator   = std::function<chr_v(evo_t&, chr_t const& chr)>; \
  \
  using creator    = std::function<chr_t(evo_t&)>; \
  using evaluator  = std::function<fit_t(evo_t&, chr_t&)>; \
  using comparator = std::function<bool(evo_t&, fit_t const&, fit_t const&)>; \
  using subtractor = std::function<dis_t(evo_t&, fit_t const&, fit_t const&)>; \
  \
  using simple_creator    = std::function<chr_t()>; \
  using simple_evaluator  = std::function<fit_t(chr_t&)>; \
  using simple_comparator = std::function<bool(fit_t const&, fit_t const&)>; \
  using simple_subtractor = std::function<dis_t(fit_t const&, fit_t const&)>
