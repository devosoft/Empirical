//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <functional>
#include <map>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../meta/reflection.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  template <typename ORG, typename GENOTYPE=ORG>
  class World {
  private:
    using ptr_t = Ptr<ORG>;
    using pop_t = emp::vector<ptr_t>;

    using fun_calc_fitness_t = std::function<double(ORG&)>;
    using fun_add_org_t      = std::function<size_t(Ptr<ORG>)>;
    using fun_add_birth_t    = std::function<size_t(Ptr<ORG>, size_t)>;

    // Main member variables
    Ptr<Random> random_ptr;         // Random object to use.
    pop_t pop;                      // All of the spots in the population.
    size_t num_orgs;                // How many organisms are actually in the population.
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; uncached values == 0.

    // Boolean values...
    bool random_owner;        // Did we create our own random number generator?
    bool cache_on;            // Should we be caching fitness values?

    // Configurable functions.
    fun_calc_fitness_t fun_calc_fitness;  // Fitness function
    fun_add_org_t      fun_add_org;       // Technique to inject a new organism.
    fun_add_birth_t    fun_add_birth;     // Technique to add a new offspring.


    // AddOrgAt & AddOrgAppend are the only ways to add organisms (others must go through these)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos);
    size_t AddOrgAppend(Ptr<ORG> new_org);

    // Default Add functions that will be updated.
    size_t AddOrgBirth_Default(Ptr<ORG> new_org, size_t parent_pos);

    // Build a Setup function in world that calls ::Setup() on whatever is passed in IF it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);

    // Other private functions:
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }

  public:
    World()
      : random_ptr(NewPtr<Random>()), pop(), num_orgs(0)
      , random_owner(true), cache_on(false)
      , fun_calc_fitness(), fun_add_org(), fun_add_birth()
    {
      fun_add_org = [this](Ptr<ORG> new_org) { return AddOrgAppend(new_org); };

      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_pos) {
        return AddOrgBirth_Default(new_org, parent_pos);
      };
    }
    ~World() {
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
    // No non-const version.
    const ORG & operator[](size_t pos) const {
      emp_assert(pop[pos] != nullptr);  // Should not index to a null organism!
      return *(pop[pos]);
    }


    // --- Calculate Fitness ---

    void SetFitFun(const fun_calc_fitness_t & fit_fun) { fun_calc_fitness = fit_fun; }

    // When calculating fitness, the three relevant inputs are the organism, the fitness function,
    // and the position in the population.
    double CalcFitnessOrg(ORG & org) { emp_assert(fun_calc_fitness); return fun_calc_fitness(org); }
    double CalcFitnessID(size_t id);

    void CalcFitnessAll() const {
      emp_assert(cache_on, "Trying to calculate fitness of all orgs without caching.");
      for (size_t id = 0; id < pop.size(); id++) CalcFitnessID(id);
    }

    void SetCache(bool _in=true) { cache_on = _in; }
    void ClearCache() { fit_cache.resize(0); }

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


    // --- SELECTION MECHANISMS ---
    // Elite Selection picks a set of the most fit individuals from the population to move to
    // the next generation.  Find top e_count individuals and make copy_count copies of each.
    void EliteSelect(size_t e_count=1, size_t copy_count=1);

  };


  // === Out-of-class member function definitions from above ===

  template<typename ORG, typename GENOTYPE>
  size_t World<ORG,GENOTYPE>::AddOrgAt(Ptr<ORG> new_org, size_t pos) {
    emp_assert(pos < pop.size());   // Make sure we are placing into a legal position.
    if (pop[pos]) { pop[pos].Delete(); --num_orgs; }
    pop[pos] = new_org;
    ++num_orgs;
    return pos;
  }

  template<typename ORG, typename GENOTYPE>
  size_t World<ORG,GENOTYPE>::AddOrgAppend(Ptr<ORG> new_org) {
    const size_t pos = pop.size();
    pop.push_back(new_org);
    ++num_orgs;
    return pos;
  }

  template<typename ORG, typename GENOTYPE>
  size_t World<ORG,GENOTYPE>::AddOrgBirth_Default(Ptr<ORG> new_org, size_t parent_pos) {
    emp_assert(random_ptr); // Random must be set before being used.
    const size_t pos = random_ptr->GetUInt(pop.size());
    return AddOrgAt(new_org, pos);
  }

  template<typename ORG, typename GENOTYPE>
  double World<ORG,GENOTYPE>::CalcFitnessID(size_t id) {
    if (!cache_on) return CalcFitnessOrg(*pop[id]);
    double cur_fit = GetCache(id);
    if (cur_fit == 0.0 && pop[id]) {   // If org is non-null, but no cached fitness, calculate it!
      if (id >= fit_cache.size()) fit_cache.resize(id+1, 0.0);
      cur_fit = CalcFitnessOrg(*pop[id]);
      fit_cache[id] = cur_fit;
    }
    return cur_fit;
  }

  // Delete all organisms.
  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::Clear() {
    for (ptr_t org : pop) if (org) org.Delete();  // Delete current organisms.
    pop.resize(0);                                // Remove deleted organisms.
    num_orgs = 0;
  }

  // Delete organism at a specified position.
  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::ClearOrgAt(size_t pos) {
    if (!pop[pos]) return;  // No organism; no need to do anything.
    pop[pos].Delete();
    pop[pos]=nullptr;
    num_orgs--;
  }

  template <typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::Insert(const ORG & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      //const size_t pos =
      fun_add_org(new_org);
      //SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template <typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::InsertAt(const ORG & mem, const size_t pos) {
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    AddOrgAt(new_org, pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG, typename GENOTYPE>
  template <typename... ARGS>
  void World<ORG,GENOTYPE>::InsertRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InsertRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    // const size_t pos =
    AddOrg(new_org);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      // const size_t pos =
      fun_add_birth(new_org, parent_pos);
      // SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::SetRandom(Random & r) {
    if (random_owner) random_ptr.Delete();
    random_ptr = &r;
    random_owner = false;
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::NewRandom(int seed) {
    if (random_owner) random_ptr.Delete();
    random_ptr.New(seed);
    random_owner = true;
  }

  // Get random *occupied* cell.
  template<typename ORG, typename GENOTYPE>
  size_t World<ORG,GENOTYPE>::GetRandomOrgID() {
    emp_assert(num_orgs > 0); // Make sure it's possible to find an organism!
    size_t pos = random_ptr->GetUInt(0, pop.size());
    while (pop[pos] == nullptr) pos = random_ptr->GetUInt(0, pop.size());
    return pos;
  }

  // Find ALL cell IDs the return true in the filter.
  template<typename ORG, typename GENOTYPE>
  emp::vector<size_t> World<ORG,GENOTYPE>::FindCellIDs(const std::function<bool(ORG*)> & filter) {
    emp::vector<size_t> valid_IDs(0);
    for (size_t i = 0; i < pop.size(); i++) {
      if (filter(pop[i].Raw())) valid_IDs.push_back(i);
    }
    return valid_IDs;
  }

  // Run population through a bottleneck to (potentially) shrink it.
  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::DoBottleneck(const size_t new_size, bool choose_random) {
    if (new_size >= pop.size()) return;  // No bottleneck needed!

    // If we are supposed to keep only random organisms, shuffle the beginning into place!
    if (choose_random) emp::Shuffle<ptr_t>(*random_ptr, pop, new_size);

    // Clear out all of the organisms we are removing and resize the population.
    for (size_t i = new_size; i < pop.size(); ++i) ClearOrgAt(i);
    pop.resize(new_size);
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::Print(std::function<std::string(ORG*)> string_fun,
			       std::ostream & os, std::string empty, std::string spacer) {
    for (ptr_t org : pop) {
      if (org) os << string_fun(org.Raw());
      else os << empty;
      os << spacer;
    }
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::Print(std::ostream & os, std::string empty, std::string spacer) {
    Print( [](ORG * org){return emp::to_string(*org);}, os, empty, spacer);
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::PrintOrgCounts(std::function<std::string(ORG*)> string_fun,
                                        std::ostream & os) {
    std::map<ORG,size_t> org_counts;
    for (ptr_t org : pop) if (org) org_counts[*org] = 0;  // Initialize needed entries
    for (ptr_t org : pop) if (org) org_counts[*org] += 1; // Count actual types.
    for (auto x : org_counts) {
      ORG cur_org = x.first;
      os << string_fun(&cur_org) << " : " << x.second << std::endl;
    }
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::EliteSelect(size_t e_count, size_t copy_count) {
    emp_assert(fun_calc_fitness);
    emp_assert(e_count > 0 && e_count <= pop.size());
    emp_assert(copy_count > 0);

    // Load the population into a multimap, sorted by fitness.
    std::multimap<double, size_t> fit_map;
    for (size_t i = 0; i < pop.size(); i++) {
      if (IsOccupied(i)) {
        const double cur_fit = CalcFitnessID(i);
        fit_map.insert( std::make_pair(cur_fit, i) );
      }
    }

    // Grab the top fitnesses and move them into the next generation.
    auto m = fit_map.rbegin();
    for (size_t i = 0; i < e_count; i++) {
      this->InsertBirth( *(pop[m->second]), m->second, copy_count);
      ++m;
    }
  }

}

#endif
