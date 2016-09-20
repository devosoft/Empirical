//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in fitness managers for use with emp::evo::World; a fitness manager
//  is in charge of determining when and how to store fitness values.
//  - Can fitness values be maintained for an organism DURING a set of selection steps?
//  - Are fitness values consistant across an entire genotype for a run?
//  - Do we need to maintain fitness values for roulette selection?

#ifndef EMP_EVO_FITNESS_MANAGER_H
#define EMP_EVO_FITNESS_MANAGER_H

#include "../tools/WeightedSet.h"

namespace emp {
namespace evo {

  class FitnessManager_Base {
  protected:
    WeightedSet roulette_info; // Data structure to use for roulette selection.

  public:
    static constexpr bool emp_is_fitness_manager = true;

    // All caching functions should get ignored at compile time.
    static constexpr bool HasCache(size_t id) { return false; }
    static constexpr double GetCache(size_t id) { return 0.0; }
    static constexpr size_t GetCacheSize() { return 0.0; }

    template <typename ORG>
    static double CalcFitness(int id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      return fit_fun(org);
    }

    static constexpr bool SetCache(size_t, double) { return false; }
    static constexpr bool ClearCache() { return false; }
    static constexpr bool ClearCache(size_t) { return false; }
    static constexpr bool ResizeCache(size_t) { return false; }
    static constexpr bool ResizeCache(size_t, double) { return false; }
  };

  class FitnessManager_CacheOrg : public FitnessManager_Base {
  protected:
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; invalid values == 0.

  public:
    bool HasCache(size_t id) const { return (id < fit_cache.size()) && (fit_cache[id] >= 0.0); }
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }
    size_t GetCacheSize() const { return fit_cache.size(); }

    template <typename ORG>
    double CalcFitness(int id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      double cur_fit = GetCache(id);
      if (!cur_fit) {
        cur_fit = fit_fun(org);
        fit_cache[id] = cur_fit;
      }
      return cur_fit;
    }

    void SetCache(size_t id, double fitness) { fit_cache[id] = fitness; }
    void ClearCache() { fit_cache.resize(0); }
    void ClearCache(size_t id) { if (id < fit_cache.size()) fit_cache[id] = 0.0; }
    void ResizeCache(size_t new_size) { fit_cache.resize(new_size); }
    void ResizeCache(size_t new_size, double def_val) { fit_cache.resize(new_size, def_val); }
  };

  using CacheOff = FitnessManager_Base;
  using CacheOrgs = FitnessManager_CacheOrg;
}
}


#endif
