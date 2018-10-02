#pragma once

#include <filesystem>
#include <fstream>
#include "base.hh"
#include "nsga/base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class nsais : public nsga<__EVO_TMPL_CLASS_ARGS> {

   public:
    __EVO_USING_TYPES(nsais);
    __EVO_USING_FUNCTIONS;

    using const_fro_iterator = siz_t const*;
    using const_dis_iterator = dis_t const*;

   protected:
    siz_t _cs;

   public:
    nsais (siz_t popsize, siz_t cs, siz_t dimensions, siz_t seed)
      : nsga<__EVO_TMPL_CLASS_ARGS>(popsize, popsize * cs, dimensions, seed), _cs(cs) {}

    evo_t* copy (void) const override { return new nsais(*this); }

    siz_t cs (void) const { return this->_cs; }

    void set_mutator (mutator const& mutate) {
      this->set_generator(typename generators::clonal(
        mutate, this->cs(), [ this ] (evo_t&, siz_t pos, siz_t) {
          return this->front_at(pos) + 1;
        }
      ));
    }

  };
}
