//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_MODULE_H
#define EMP_EVO_WORLD_MODULE_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  enum class evo {
    EA,       // Adds vector for next generation + changes "next" pointer to new vector

    Mixed,    // Well-mixed population structure.
    Grid,     // Grid population structure.
    Pools,    // Multiple pools population structure.

    Genotype, // Organisms have a genotype distinct from the organism. @CAO: Auto-detect?

    Environment, // Setup environmental resources (for use in fitness calculations.)

    CalcFit,  // Calculate fitness whenever it is needed.
    CacheFit, // Cache fitness for each organism.
    TrackFit, // Calculate all fitness at birth; track weighted ratios.

    Stats,    // Add extra features to track what's going on in the world. @CAO: Multiple versions?
    Lineage,  // Calculate the full phylogeny as populations continue.

    Signals,  // Tie signals into any of the above functions to override them in a more dynamic way.

    UNKNOWN   // Unknown modifier; will trigger error.
  };

  // Generic form of WorldModule (should never be used; trigger error!)
  template <typename VAL_TYPE, emp::evo... MODS> class WorldModule {
  public:
    WorldModule() { emp_assert(false, "Unknown module used in World!"); }
  };

  // == World BASE class ==
  //
  // This class is always build on for other worlds.

  template <typename ORG>
  class WorldModule<ORG> {
  protected:
    using ptr_t = Ptr<ORG>;
    using pop_t = emp::vector<ptr_t>;
    using fit_fun_t = std::function<double(ORG*)>;

    Ptr<Random> random_ptr;  // Random object to use.
    bool random_owner;       // Did we create our own random number generator?
    pop_t pop;               // All of the spots in the population.
    size_t num_orgs;         // How many organisms are actually in the population.

    // AddOrgAt & AddOrgAppend are the only ways to add organisms (others must go through these)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos);
    size_t AddOrgAppend(Ptr<ORG> new_org);

    // The following functions call focus on context (and will be overridden as needed).
    // AddOrg inserts an organism from OUTSIDE of the population.
    // AddOrgBirth inserts an organism that was born INSIDE the population.
    size_t AddOrg(Ptr<ORG> new_org) { return AddOrgAppend(new_org); }

    size_t AddOrgBirth(Ptr<ORG> new_org, size_t parent_pos) {
      emp_assert(random_ptr); // Random must be set before being used.
      const size_t pos = random_ptr->GetUInt(pop.size());
      return AddOrgAt(new_org, pos);
    }

  public:
    WorldModule() : random_ptr(nullptr), random_owner(false), num_orgs(0) { ; }
    ~WorldModule() {
      Clear();
      if (random_owner) random_ptr.Delete();
    }

    // --- Publicly available types ---

    using value_type = ORG;


    // --- Accessing Organisms or info ---

    size_t GetSize() const { return pop.size(); }
    size_t GetNumOrgs() const { return num_orgs; }
    bool IsOccupied(size_t i) const { return pop[i] != nullptr; }

    ORG & operator[](size_t pos) { return *(pop[pos]); }
    const ORG & operator[](size_t pos) const { return *(pop[pos]); }

    // --- MANIPULATE ORGS IN POPULATION ---

    void Clear();
    void ClearOrgAt(size_t pos);

    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pop.size(); i++) ClearOrgAt(i); // Remove orgs past new size.
      pop.resize(new_size, nullptr);                                // Default new orgs to null.
    }


    // --- RANDOM FUNCTIONS ---

    Random & GetRandom() { return *random_ptr; }

    // Set or create a new random number generator.
    void SetRandom(Random & r);
    void NewRandom(int seed=-1);

    // Get any cell, at random.
    size_t GetRandomCellID() { return random_ptr->GetInt(0, pop.size()); }

    // By default, assume a well-mixed population so random neighbors can be anyone.
    size_t GetRandomNeighborID(size_t /*id*/) { return random_ptr->GetUInt(0, pop.size()); }

    // Get random *occupied* cell.
    size_t GetRandomOrgID();


    // --- POPULATION ANALYSIS ---

    // Find ALL cell IDs the return true in the filter.
    emp::vector<size_t> FindCellIDs(const std::function<bool(ORG*)> & filter);

    // Simple techniques for using FindCellIDs()
    emp::vector<size_t> GetValidOrgIDs() {
      return FindCellIDs([](ORG*org){ return org != nullptr; });
    }
    emp::vector<size_t> GetEmptyPopIDs() {
      return FindCellIDs([](ORG*org){ return org == nullptr; });
    }


    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentiall) shrink it.
    void DoBottleneck(const size_t new_size, bool choose_random=true) {
      if (new_size >= pop.size()) return;  // No bottleneck needed!

      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

      // Clear out all of the organisms we are removing and resize the population.
      for (size_t i = new_size; i < pop.size(); ++i) ClearOrgAt(i);
      pop.resize(new_size);
    }

    // --- PRINTING ---

    void Print(std::function<std::string(ORG*)> string_fun,
	       std::ostream & os = std::cout,
	       std::string empty="X", std::string spacer=" ") {
      for (ptr_t org : pop) {
        if (org) os << string_fun(org.Raw());
        else os << empty;
        os << spacer;
      }
    }
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      Print( [](ORG * org){return emp::to_string(*org);}, os, empty, spacer);
    }
    void PrintOrgCounts(std::function<std::string(ORG*)> string_fun,
			std::ostream & os = std::cout) {
      std::map<ORG,size_t> org_counts;
      for (ptr_t org : pop) if (org) org_counts[*org] = 0;  // Initialize needed entries
      for (ptr_t org : pop) if (org) org_counts[*org] += 1; // Count actual types.
      for (auto x : org_counts) {
        ORG cur_org = x.first;
        os << string_fun(&cur_org) << " : " << x.second << std::endl;
      }
    }

    // --- FOR VECTOR COMPATIBILITY ---
    size_t size() const { return pop.size(); }
    void resize(size_t new_size) { Resize(new_size); }
    void clear() { Clear(); }

//     Proxy operator[](size_t i) { return Proxy(*this, i); }
//     const ptr_t operator[](size_t i) const { return pop[i]; }
//     iterator_t begin() { return iterator_t(this, 0); }
//     iterator_t end() { return iterator_t(this, (int) pop.size()); }
  };


  // --- Member functions from above ---

  template<typename ORG>
  size_t WorldModule<ORG>::AddOrgAt(Ptr<ORG> new_org, size_t pos) {
    emp_assert(pos < pop.size());   // Make sure we are placing into a legal position.
    if (pop[pos]) { pop[pos].Delete(); --num_orgs; }
    pop[pos] = new_org;
    ++num_orgs;
    return pos;
  }

  template<typename ORG>
  size_t WorldModule<ORG>::AddOrgAppend(Ptr<ORG> new_org) {
    const size_t pos = pop.size();
    pop.push_back(new_org);
    ++num_orgs;
    return pos;
  }

  // Delete all organisms.
  template<typename ORG>
  void WorldModule<ORG>::Clear() {
    for (ptr_t org : pop) if (org) org.Delete();  // Delete current organisms.
    pop.resize(0);                                // Remove deleted organisms.
    num_orgs = 0;
  }

  // Delete organism at a specified position.
  template<typename ORG>
  void WorldModule<ORG>::ClearOrgAt(size_t pos) {
    if (!pop[pos]) return;  // No organism; no need to do anything.
    pop[pos].Delete();
    pop[pos]=nullptr;
    num_orgs--;
  }

  template<typename ORG>
  void WorldModule<ORG>::SetRandom(Random & r) {
    if (random_owner) random_ptr.Delete();
    random_ptr = &r;
    random_owner = false;
  }

  template<typename ORG>
  void WorldModule<ORG>::NewRandom(int seed) {
    if (random_owner) random_ptr.Delete();
    random_ptr.New(seed);
    random_owner = true;
  }

  // Get random *occupied* cell.
  template<typename ORG>
  size_t WorldModule<ORG>::GetRandomOrgID() {
    emp_assert(num_orgs > 0); // Make sure it's possible to find an organism!
    size_t pos = random_ptr->GetUInt(0, pop.size());
    while (pop[pos] == nullptr) pos = random_ptr->GetUInt(0, pop.size());
    return pos;
  }

  // Find ALL cell IDs the return true in the filter.
  template<typename ORG>
  emp::vector<size_t> WorldModule<ORG>::FindCellIDs(const std::function<bool(ORG*)> & filter) {
    emp::vector<size_t> valid_IDs(0);
    for (size_t i = 0; i < pop.size(); i++) {
      if (filter(pop[i].Raw())) valid_IDs.push_back(i);
    }
    return valid_IDs;
  }
}

#endif
