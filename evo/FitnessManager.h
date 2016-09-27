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

  public:
    static constexpr bool emp_is_fitness_manager = true;

    // All caching functions should get ignored at compile time.
    static constexpr double GetCache(size_t id) { return 0.0; }
    static constexpr size_t GetSize() { return 0.0; }

    template <typename ORG>
    static double CalcFitness(int id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      return fit_fun(org);
    }

    static constexpr bool Set(const emp::vector<double> &) { return false; }
    static constexpr bool SetID(size_t, double) { return false; }
    static constexpr bool Clear() { return false; }
    static constexpr bool ClearAt(size_t) { return false; }
    static constexpr bool Resize(size_t) { return false; }
    static constexpr bool Resize(size_t, double) { return false; }
  };

  class FitnessManager_CacheOrg : public FitnessManager_Base {
  protected:
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; invalid values == 0.

  public:
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }
    size_t GetSize() const { return fit_cache.size(); }

    template <typename ORG>
    double CalcFitness(size_t id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      double cur_fit = GetCache(id);
      if (!cur_fit) {
        if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
        cur_fit = fit_fun(org);
        fit_cache[id] = cur_fit;
      }
      return cur_fit;
    }

    bool Set(const emp::vector<double> & in_cache) { fit_cache = in_cache; return true; }
    bool SetID(size_t id, double fitness) { fit_cache[id] = fitness; return true; }
    bool Clear() { fit_cache.resize(0); return true; }
    bool ClearAt(size_t id) { if (id < fit_cache.size()) fit_cache[id] = 0.0; return true; }
    bool Resize(size_t new_size) { fit_cache.resize(new_size); return true; }
    bool Resize(size_t new_size, double def_val) { fit_cache.resize(new_size, def_val); return true; }
  };

  // FitnessManager_Proportion requires the user to maintain fitness.
  class FitnessManager_Weights : public FitnessManager_Base {
  protected:
    WeightedSet weight_info; // Data structure to use for roulette selection.

  public:
    double GetCache(size_t id) const { return weight_info[id]; }
    size_t GetSize() const { return weight_info.size(); }

    template <typename ORG>
    double CalcFitness(size_t id, ORG*, const std::function<double(ORG*)> &) {
      // We shouldn't need to call this version frequently; it should always return cache info.
      return weight_info[id];
    }

    bool Set(const emp::vector<double> & in_cache) { weight_info.Adjust(in_cache); return true; }
    bool SetID(size_t id, double fitness) { weight_info.Adjust(id,fitness); return true; }
    bool Clear() { weight_info.Clear(); return true; }
    bool ClearAt(size_t id) { weight_info.Adjust(id, 0.0); return true; }
    bool Resize(size_t new_size) { weight_info.Resize(new_size); return true; }
    bool Resize(size_t new_size, double def_val) { weight_info.Resize(new_size, def_val); return true; }
  };


  using CacheOff = FitnessManager_Base;
  using CacheOrgs = FitnessManager_CacheOrg;
  using FitWeights = FitnessManager_Weights;
}
}


#endif
