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
    using parent_t::default_fit_fun;
    using parent_t::pop;

    // Parent member functions
    using parent_t::IsOccupied;

    // Caching-specific members.
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; invalid values == 0.

    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }
  public:

    // Override fitness calculation methods
    double CalcFitnessOrg(ORG & org, const fit_fun_t & fit_fun) { return fit_fun(org); }

    double CalcFitnessOrg(ORG & org) {
      emp_assert(default_fit_fun);
      return CalcFitnessOrg(org, default_fit_fun);
    }

    double CalcFitnessID(size_t id, const fit_fun_t & fun) {
      double cur_fit = GetCache(id);
      if (cur_fit == 0.0 && pop[id]) {   // If org is non-null, but no cached fitness, calculate it!
        if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
        cur_fit = parent_t::CalcFitnessOrg(*pop[id], fun);
        fit_cache[id] = cur_fit;
      }
      return cur_fit;
    }

    double CalcFitnessID(size_t id) {
      emp_assert(default_fit_fun);
      return CalcFitnessID(id, default_fit_fun);
    }

    void CalcFitnessAll(const fit_fun_t & fit_fun) const {
      for (size_t id = 0; id < pop.size(); id++) CalcFitnessID(id, fit_fun);
    }

  };


}

#endif
