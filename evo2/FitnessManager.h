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

#include <unordered_map>

#include "../tools/memo_function.h"
#include "../tools/WeightedSet.h"

namespace emp {
namespace evo {

  template <typename ORG, typename WM> // Collect ORG and WorldManager info for consistency, even though not needed.
  class FitnessManager_Base : public NextWorldManager<ORG, WM> {
  protected:

  public:
    static constexpr bool emp_is_fitness_manager = true;

    // All caching functions should get ignored at compile time.
    static constexpr double GetCache(size_t id) { return 0.0; }
    static constexpr size_t GetCacheSize() { return 0; }

    template <typename ORG>
    static double CalcFitness(size_t id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      return org ? fit_fun(org) : 0.0;
    }
    template <typename ORG>
    static double CalcFitness(size_t id, ORG* org, emp::memo_function<double(ORG*)> & fit_fun) {
      return org ? fit_fun(org) : 0.0;
    }

    static constexpr bool SetFitness(const emp::vector<double> &) { return false; }
    static constexpr bool SetIDFitness(size_t, double) { return false; }
    static constexpr bool ClearFitness() { return false; }               // Clear all cache
    static constexpr bool ClearFitnessAt(size_t) { return false; }       // Clear cache for specific org
    static constexpr bool ClearPopFitness() { return false; }            // Clear cache for all orgs
    static constexpr bool ResizeFitness(size_t) { return false; }
    static constexpr bool ResizeFitness(size_t, double) { return false; }

    static constexpr bool IsCached() { return false; } // Is this FitnessManager_Cached or Tracker?
    static constexpr bool IsTracked() { return false; } // Is this FitnessManager_Tracker?

    // These functions only work properly in FitnessManager_Tracker...
    static double GetTotalFitness() { emp_assert(false, "Use FitnessManager_Tracker"); return 0.0; }
    static size_t AtFitness(double index) { emp_assert(false, "Use FitnessManager_Tracker"); return 0; }
  };

  template <typename ORG, typename WM>
  class FitnessManager_CacheOrg : public FitnessManager_Base<ORG,WM> {
  protected:
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; invalid values == 0.

  public:
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }
    size_t GetCacheSize() const { return fit_cache.size(); }

    template <typename ORG>
    double CalcFitness(size_t id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      double cur_fit = GetCache(id);
      if (cur_fit == 0.0 && org) {    // If org is non-null, but no cached fitness, calculate it!
        if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
        cur_fit = fit_fun(org);
        fit_cache[id] = cur_fit;
      }
      return cur_fit;
    }
    template <typename ORG>
    double CalcFitness(size_t id, ORG* org, emp::memo_function<double(ORG*)> & fit_fun) {
      double cur_fit = GetCache(id);
      if (!cur_fit && org) {    // If org is non-null, but no cached fitness, calculate it!
        if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
        cur_fit = fit_fun(org);
        fit_cache[id] = cur_fit;
      }
      return cur_fit;
    }

    bool SetFitness(const emp::vector<double> & in_cache) { fit_cache = in_cache; return true; }
    bool SetIDFitness(size_t id, double fitness) { fit_cache[id] = fitness; return true; }
    bool ClearFitness() { fit_cache.resize(0); return true; }
    bool ClearFitnessAt(size_t id) { if (id < fit_cache.size()) fit_cache[id] = 0.0; return true; }
    bool ClearPopFitness() { fit_cache.resize(0); return true; }
    bool ResizeFitness(size_t new_size) { fit_cache.resize(new_size); return true; }
    bool ResizeFitness(size_t new_size, double def_val) { fit_cache.resize(new_size, def_val); return true; }

    static constexpr bool IsCached() { return true; } // Is this FitnessManager_Cached or Tracker?
  };


  // FitnessManager_Proportion requires the user to maintain fitness.
  template <typename ORG, typename WM>
  class FitnessManager_Tracker : public FitnessManager_Base<ORG, WM> {
  protected:
    WeightedSet weight_info; // Data structure to use for roulette selection.

  public:
    double GetCache(size_t id) const { return weight_info[id]; }
    size_t GetCacheSize() const { return weight_info.size(); }

    template <typename ORG>
    double CalcFitness(size_t id, ORG* org, const std::function<double(ORG*)> & fit_fun) {
      // Organisms that don't exist should have a zero fitness.
      if (!org) return 0.0;

      // If we don't have a fitness cached calculate it an PUT IT IN THE CACHE.
      if (weight_info.GetWeight(id) == 0.0) {
        weight_info[id] = fit_fun(org);
      }

      // Return the fitness in the cache.
      return weight_info.GetWeight(id);
    }

    bool SetFitness(const emp::vector<double> & in_cache) { weight_info.Adjust(in_cache); return true; }
    bool SetIDFitness(size_t id, double fitness) { weight_info.Adjust(id,fitness); return true; }
    bool ClearFitness() { weight_info.Clear(); return true; }
    bool ClearFitnessAt(size_t id) { weight_info.Adjust(id, 0.0); return true; }
    bool ClearPopFitness() { weight_info.Clear(); return true; }
    bool ResizeFitness(size_t new_size) { weight_info.Resize(new_size); return true; }
    bool ResizeFitness(size_t new_size, double def_val) { weight_info.Resize(new_size, def_val); return true; }

    static constexpr bool IsCached() { return true; } // Is this FitnessManager_Cached or Tracker?
    static constexpr bool IsTracked() { return true; } // Is this FitnessManager_Tracker?

    double GetTotalFitness() const { return weight_info.GetWeight(); }
    size_t AtFitness(double index) const { return weight_info.Index(index); }
  };


  using FitCacheOff = FitnessManager_Base;
  using FitCacheOn = FitnessManager_CacheOrg;
  using FitTrack = FitnessManager_Tracker;
}
}


#endif
