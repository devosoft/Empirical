//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in fitness managers for use with emp::evo::World; a fitness manager
//  is in charge of determining when and how to store fitness values.
//  - Can fitness values be maintained for an organism DURING a set of selection steps?
//  - Are fitness values consistant across an entire genotype for a run?
//  - Do we need to maintain fitness values for all organisms (e.g., for roulette selection)?
//
//  Note: FitnessManagers can also take memo_functions for a different type of caching.

#ifndef EMP_EVO_FITNESS_MANAGER_H
#define EMP_EVO_FITNESS_MANAGER_H

#include <unordered_map>

#include "../tools/memo_function.h"
#include "../tools/IndexMap.h"

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
    static double CalcFitness(size_t id, Ptr<ORG> org, const std::function<double(ORG*)> & fit_fun) {
      return org ? fit_fun(org) : 0.0;
    }
    template <typename ORG>
    static double CalcFitness(size_t id, Ptr<ORG> org, emp::memo_function<double(ORG*)> & fit_fun) {
      return org ? fit_fun(org) : 0.0;
    }

    static constexpr bool Set(const emp::vector<double> &) { return false; }
    static constexpr bool SetID(size_t, double) { return false; }
    static constexpr bool Clear() { return false; }               // Clear all cache
    static constexpr bool ClearAt(size_t) { return false; }       // Clear cache for specific org
    static constexpr bool ClearPop() { return false; }            // Clear cache for all orgs
    static constexpr bool Resize(size_t) { return false; }
    static constexpr bool Resize(size_t, double) { return false; }

    static constexpr bool IsCached() { return false; } // Is this FitnessManager_Cached or Tracker?
    static constexpr bool IsTracked() { return false; } // Is this FitnessManager_Tracker?

    // These functions only work properly in FitnessManager_Tracker...
    static double GetTotalFitness() { emp_assert(false, "Use FitnessManager_Tracker"); return 0.0; }
    static size_t At(double index) { emp_assert(false, "Use FitnessManager_Tracker"); return 0; }
  };

  class FitnessManager_CacheOrg : public FitnessManager_Base {
  protected:
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; invalid values == 0.

  public:
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }
    size_t GetSize() const { return fit_cache.size(); }

    template <typename ORG>
    double CalcFitness(size_t id, Ptr<ORG> org, const std::function<double(ORG*)> & fit_fun) {
      double cur_fit = GetCache(id);
      if (cur_fit == 0.0 && org) {    // If org is non-null, but no cached fitness, calculate it!
        if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
        cur_fit = fit_fun(org);
        fit_cache[id] = cur_fit;
      }
      return cur_fit;
    }
    template <typename ORG>
    double CalcFitness(size_t id, Ptr<ORG> org, emp::memo_function<double(ORG*)> & fit_fun) {
      double cur_fit = GetCache(id);
      if (!cur_fit && org) {    // If org is non-null, but no cached fitness, calculate it!
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
    bool ClearPop() { fit_cache.resize(0); return true; }
    bool Resize(size_t new_size) { fit_cache.resize(new_size); return true; }
    bool Resize(size_t new_size, double def_val) { fit_cache.resize(new_size, def_val); return true; }

    static constexpr bool IsCached() { return true; } // Is this FitnessManager_Cached or Tracker?
  };


  // FitnessManager_Proportion requires the user to maintain fitness.
  class FitnessManager_Tracker : public FitnessManager_Base {
  protected:
    IndexMap index_info; // Data structure to use for roulette selection.

  public:
    double GetCache(size_t id) const { return index_info[id]; }
    size_t GetSize() const { return index_info.size(); }

    template <typename ORG>
    double CalcFitness(size_t id, Ptr<ORG> org, const std::function<double(ORG*)> & fit_fun) {
      // Organisms that don't exist should have a zero fitness.
      if (!org) return 0.0;

      // If we don't have a fitness cached calculate it and PUT IT IN THE CACHE.
      if (index_info.GetWeight(id) == 0.0) {
        index_info[id] = fit_fun(org);
      }

      // Return the fitness in the cache.
      return index_info.GetWeight(id);
    }

    bool Set(const emp::vector<double> & in_cache) { index_info.Adjust(in_cache); return true; }
    bool SetID(size_t id, double fitness) { index_info.Adjust(id,fitness); return true; }
    bool Clear() { index_info.Clear(); return true; }
    bool ClearAt(size_t id) { index_info.Adjust(id, 0.0); return true; }
    bool ClearPop() { index_info.Clear(); return true; }
    bool Resize(size_t new_size) { index_info.Resize(new_size); return true; }
    bool Resize(size_t new_size, double def_val) { index_info.Resize(new_size, def_val); return true; }

    static constexpr bool IsCached() { return true; } // Is this FitnessManager_Cached or Tracker?
    static constexpr bool IsTracked() { return true; } // Is this FitnessManager_Tracker?

    double GetTotalFitness() const { return index_info.GetWeight(); }
    size_t At(double index) const { return index_info.Index(index); }
  };

  // FitnessManager_Proportion requires the user to maintain fitness.
  class FitnessManager_Dynamic : public FitnessManager_Base {
  protected:
    enum CacheType { CACHE_OFF, CACHE_ON, TRACK_ON };
    CacheType cache_type = CACHE_OFF;

    emp::vector<double> fit_cache;  // vector size == 0 when not caching; invalid values == 0.
    IndexMap index_info;            // Data structure to use for roulette selection.

  public:
    double GetCache(size_t id) const {
      switch (cache_type) {
      case CACHE_OFF: return 0.0;
      case CACHE_ON:  return (id < fit_cache.size()) ? fit_cache[id] : 0.0;
      case TRACK_ON:  return index_info[id];
      }
    }
    size_t GetSize() const {
      switch (cache_type) {
      case CACHE_OFF: return 0;
      case CACHE_ON:  return fit_cache.size();
      case TRACK_ON:  return index_info.size();
      }
    }

    template <typename ORG>
    double CalcFitness(size_t id, Ptr<ORG> org, const std::function<double(ORG*)> & fit_fun) {
      // Organisms that don't exist should have a zero fitness.
      if (!org) return 0.0;

      // If we don't have a fitness cached calculate it and cache if needed.
      switch (cache_type) {
      case CACHE_OFF:
        return fit_fun(org);;
      case CACHE_ON:
        if (GetCache(id) == 0.0) {                // If no cached fitness, calculate it!
          if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
          fit_cache[id] = fit_fun(org);
        }
        return GetCache(id);
      case TRACK_ON:
        if (index_info.GetWeight(id) == 0.0) { index_info[id] = fit_fun(org); }
        return index_info.GetWeight(id);
      }
    }


    bool Set(const emp::vector<double> & in_cache) {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  fit_cache = in_cache; return true;
        case TRACK_ON:  index_info.Adjust(in_cache); return true;
      }
    }

    bool SetID(size_t id, double fitness) {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  fit_cache[id] = fitness; return true;
        case TRACK_ON:  index_info.Adjust(id,fitness); return true;
      }
    }

    bool Clear() {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  fit_cache.resize(0); return true;
        case TRACK_ON:  index_info.Clear(); return true;
      }
    }

    bool ClearAt(size_t id) {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  if (id < fit_cache.size()) fit_cache[id] = 0.0; return true;
        case TRACK_ON:  index_info.Adjust(id, 0.0); return true;
      }
    }

    bool ClearPop() {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  fit_cache.resize(0); return true;
        case TRACK_ON:  index_info.Clear(); return true;
      }
    }

    bool Resize(size_t new_size) {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  fit_cache.resize(new_size); return true;
        case TRACK_ON:  index_info.Resize(new_size); return true;
      }
    }

    bool Resize(size_t new_size, double def_val) {
      switch (cache_type) {
        case CACHE_OFF: return false;
        case CACHE_ON:  fit_cache.resize(new_size, def_val); return true;
        case TRACK_ON:  index_info.Resize(new_size, def_val); return true;
      }
    }


    bool IsCached() { return cache_type == CACHE_ON || cache_type == TRACK_ON; }
    bool IsTracked() { return cache_type == TRACK_ON; }

    double GetTotalFitness() const {
      emp_assert(cache_type == TRACK_ON);
      return index_info.GetWeight();
    }
    size_t At(double index) const {
      emp_assert(cache_type == TRACK_ON);
      return index_info.Index(index);
    }

    void CacheOff() { cache_type = CACHE_OFF; Resize(0, 0.0); }
    void CacheOn() { cache_type = CACHE_ON; Resize(0, 0.0); }
    void TrackOn() { cache_type = TRACK_ON; Resize(0, 0.0); }
  };

  using FitCacheOff = FitnessManager_Base;
  using FitCacheOn = FitnessManager_CacheOrg;
  using FitTrack = FitnessManager_Tracker;
  using FitDynamic = FitnessManager_Dynamic;
}
}


#endif
