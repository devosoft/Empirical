//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_MODULE_H
#define EMP_EVO_WORLD_MODULE_H

#include <map>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../meta/reflection.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"
#include "../tools/string_utils.h"

namespace emp {

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
    using fit_fun_t = std::function<double(ORG&)>;

    Ptr<Random> random_ptr;    // Random object to use.
    bool random_owner;         // Did we create our own random number generator?
    pop_t pop;                 // All of the spots in the population.
    size_t num_orgs;           // How many organisms are actually in the population.

    fit_fun_t default_fit_fun; // Fitness function to use when none explicitly provided.

    // AddOrgAt & AddOrgAppend are the only ways to add organisms (others must go through these)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos);
    size_t AddOrgAppend(Ptr<ORG> new_org);

    // The following functions call focus on context (and will be overridden as needed).
    // AddOrg inserts an organism from OUTSIDE of the population.
    // AddOrgBirth inserts an organism that was born INSIDE the population.
    size_t AddOrg(Ptr<ORG> new_org) { return AddOrgAppend(new_org); }

    size_t AddOrgBirth(Ptr<ORG> new_org, size_t parent_pos);

    // Build a Setup function in world that calls ::Setup() on whatever is passed in IF it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);

  public:
    WorldModule()
    : random_ptr(NewPtr<Random>()), random_owner(true), pop(), num_orgs(0) { ; }
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

    // We ONLY have a const index operator since manipulations should go through other functions.
    // ORG & operator[](size_t pos) { return *(pop[pos]); }
    const ORG & operator[](size_t pos) const {
      emp_assert(pop[pos] != nullptr);  // Should not index to a null organism!
      return *(pop[pos]);
    }


    // --- Calculate Fitness ---

    // When calculating fitness, the three relevant inputs are the organism, the fitness function,
    // and the position in the population.
    double CalcFitnessOrg(ORG & org, const fit_fun_t & fit_fun) { return fit_fun(org); }
    double CalcFitnessOrg(ORG & org) {
      emp_assert(default_fit_fun);
      return CalcFitnessOrg(org, default_fit_fun);
    }
    double CalcFitnessID(size_t id) { return CalcFitnessOrg(*pop[id]); }
    double CalcFitnessID(size_t id, const fit_fun_t & fun) { return CalcFitnessOrg(*pop[id], fun); }

    void CalcFitnessAll(const fit_fun_t & fit_fun) const {
      // Calculting fitness of all organisms doesn't have return values, so the only advantage is
      // if you are caching the results.
      emp_assert(false, "Trying to calculate fitness of all orgs without caching.");
    }

    // --- MANIPULATE ORGS IN POPULATION ---

    void Clear();
    void ClearOrgAt(size_t pos);

    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pop.size(); i++) ClearOrgAt(i); // Remove orgs past new size.
      pop.resize(new_size, nullptr);                                // Default new orgs to null.
    }

    // Insert an organism using the default insertion scheme.
    void Insert(const ORG & mem, size_t copy_count=1);

    // Inset and organism at a specific position.
    void InsertAt(const ORG & mem, const size_t pos);

    // Insert a random organism (constructor must facilitate!)
    template <typename... ARGS> void InsertRandomOrg(ARGS &&... args);

    // Insert a newborn by default rules, with parent information.
    void InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count=1);

    // If InsertBirth is provided with a fitness function, immediately calculate fitness of new org.
    void InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count,
                     const fit_fun_t & fit_fun);

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
    emp::vector<size_t> GetValidOrgIDs() { return FindCellIDs([](ORG*org){ return (bool) org; }); }
    emp::vector<size_t> GetEmptyPopIDs() { return FindCellIDs([](ORG*org){ return !org; }); }


    // --- POPULATION MANIPULATIONS ---

    // Run population through a bottleneck to (potentially) shrink it.
    void DoBottleneck(const size_t new_size, bool choose_random=true);


    // --- PRINTING ---

    void Print(std::function<std::string(ORG*)> string_fun, std::ostream & os = std::cout,
	       std::string empty="X", std::string spacer=" ");
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ");
    void PrintOrgCounts(std::function<std::string(ORG*)> string_fun,
			std::ostream & os = std::cout);


    // --- FOR VECTOR COMPATIBILITY ---
    size_t size() const { return pop.size(); }
    void resize(size_t new_size) { Resize(new_size); }
    void clear() { Clear(); }

//     iterator_t begin() { return iterator_t(this, 0); }
//     iterator_t end() { return iterator_t(this, (int) pop.size()); }
  };


  // === Out-of-class member function definitions from above ===

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

  template<typename ORG>
  size_t WorldModule<ORG>::AddOrgBirth(Ptr<ORG> new_org, size_t parent_pos) {
    emp_assert(random_ptr); // Random must be set before being used.
    const size_t pos = random_ptr->GetUInt(pop.size());
    return AddOrgAt(new_org, pos);
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

  template <typename ORG>
  void WorldModule<ORG>::Insert(const ORG & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      //const size_t pos =
      AddOrg(new_org);
      //SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template <typename ORG>
  void WorldModule<ORG>::InsertAt(const ORG & mem, const size_t pos) {
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    AddOrgAt(new_org, pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG>
  template <typename... ARGS>
  void WorldModule<ORG>::InsertRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InsertRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    // const size_t pos =
    AddOrg(new_org);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG>
  void WorldModule<ORG>::InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      // const size_t pos =
      AddOrgBirth(new_org, parent_pos);
      // SetupOrg(*new_org, &callbacks, pos);
    }
  }

  // If InsertBirth is provided with a fitness function, use it to calculate fitness of new org.
  template <typename ORG>
  void WorldModule<ORG>::InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count,
                                     const fit_fun_t & fit_fun) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      // const size_t pos =
      AddOrgBirth(new_org, parent_pos);
      // If we offspring are placed into the same population, recalculate fitness.
      // CalcFitness(pos, fit_fun);
      // SetupOrg(*new_org, &callbacks, pos);
    }
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

  // Run population through a bottleneck to (potentially) shrink it.
  template<typename ORG>
  void WorldModule<ORG>::DoBottleneck(const size_t new_size, bool choose_random) {
    if (new_size >= pop.size()) return;  // No bottleneck needed!

    // If we are supposed to keep only random organisms, shuffle the beginning into place!
    if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

    // Clear out all of the organisms we are removing and resize the population.
    for (size_t i = new_size; i < pop.size(); ++i) ClearOrgAt(i);
    pop.resize(new_size);
  }

  template<typename ORG>
  void WorldModule<ORG>::Print(std::function<std::string(ORG*)> string_fun,
       std::ostream & os, std::string empty, std::string spacer) {
    for (ptr_t org : pop) {
      if (org) os << string_fun(org.Raw());
      else os << empty;
      os << spacer;
    }
  }

  template<typename ORG>
  void WorldModule<ORG>::Print(std::ostream & os, std::string empty, std::string spacer) {
    Print( [](ORG * org){return emp::to_string(*org);}, os, empty, spacer);
  }

  template<typename ORG>
  void WorldModule<ORG>::PrintOrgCounts(std::function<std::string(ORG*)> string_fun,
                                        std::ostream & os) {
    std::map<ORG,size_t> org_counts;
    for (ptr_t org : pop) if (org) org_counts[*org] = 0;  // Initialize needed entries
    for (ptr_t org : pop) if (org) org_counts[*org] += 1; // Count actual types.
    for (auto x : org_counts) {
      ORG cur_org = x.first;
      os << string_fun(&cur_org) << " : " << x.second << std::endl;
    }
  }

}

#endif
