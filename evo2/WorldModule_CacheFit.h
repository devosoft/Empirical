//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Cache fitnesses for organisms so that we don't calculate more than once per organism.

#ifndef EMP_EVO_WORLD_MODULE_CACHE_FIT_H
#define EMP_EVO_WORLD_MODULE_CACHE_FIT_H

namespace emp {

  template <typename ORG, emp::evo... MODS>
  class WorldModule<ORG, evo::CacheFit, MODS...> : public WorldModule<ORG, MODS...> {
  protected:
    // Parent-related types.
    using parent_t = WorldModule<ORG, MODS...>;
    using fit_fun_t = typename parent_t::fit_fun_t;

    // Parent member vars.
    using parent_t::pop;

    // Parent member functions
    using parent_t::CalcFitness;
    using parent_t::IsOccupied;

  public:

    // Override fitness calculation methods 
    double CalcFitness(ORG & org, const fit_fun_t & fit_fun) { return fit_fun(org); }

    double CalcFitness(ORG & org) {
      emp_assert(default_fit_fun);
      return CalcFitness(org, default_fit_fun);
    }

    void CalcFitnessAll(const fit_fun_t & fit_fun) const {
      for (size_t id = 0; id < pop.size(); id++) fitM.CalcFitness(id, pop[id], fit_fun);
    }
    
  };


}

#endif
