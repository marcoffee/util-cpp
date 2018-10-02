#pragma once

#include <filesystem>
#include <fstream>
#include "base.hh"
#include "nsga/base.hh"

namespace __EVO_NAMESPACE {

  __EVO_TMPL_HEAD
  class nsais : public __EVO_CLASS(nsga) {

   public:
    __EVO_USING_TYPES(nsais);
    __EVO_USING_FUNCTIONS;

    using const_fro_iterator = siz_t const*;
    using const_dis_iterator = dis_t const*;

   protected:
    siz_t cs_;

   public:
    nsais (siz_t popsize, siz_t cs, siz_t dimensions, siz_t seed)
    : __EVO_CLASS(nsga){ popsize, popsize * cs, dimensions, seed },
      cs_{ cs } {}

    evo_t* copy (void) const override { return new nsais(*this); }

    siz_t cs (void) const { return this->cs_; }

    void set_mutator (mutator const& mutate) {
      this->set_generator(typename generators::clonal(
        mutate, this->cs(), [ this ] (evo_t&, siz_t pos, siz_t) {
          return this->front_at(pos) + 1;
        }
      ));
    }

  };
}
