//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.
//
//
//  There are three ways that organisms can enter the population:
//   * InjectAt(org, pos) - place the organism at the specified position in the population.
//   * Inject(org) - place the organism using a default postion (given other settings).
//   * DoBirth(org, parent_pos) - place the organism using current birth settings.
//
//  If the population is in EA mode (with synchronous generations), DoBirth will place offspring in
//  a "next generation" placeholder population.  Update() will move orgs into primary population.
//
//  All insertions into the popoulation funnel through private function AddOrgAt(org, pos);
//
//
//  Developer Notes:
//  * We should Specialize World so that ANOTHER world can be used with proper delegation to
//    facilitate demes, pools, islands, etc.
//  * With DoMutations, should we update taxa?  Or just assume that it will be handled
//    properly when the organisms move to the next generation.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <functional>
#include <map>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../meta/reflection.h"
#include "../tools/map_utils.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"
#include "../tools/string_utils.h"

#include "Systematics.h"
#include "World_reflect.h"  // Handle needed reflection on incoming organism classes.
#include "World_iterator.h"

namespace emp {

  template <typename ORG>
  class World {
  private:
    using this_t = World<ORG>;
    using genome_t = typename emp::find_genome_t<ORG>;
    using genotype_t = emp::Taxon<genome_t>;

    friend class World_iterator<this_t>;

    using fun_calc_fitness_t = std::function<double(ORG&)>;
    using fun_do_mutations_t = std::function<void(ORG&,Random&)>;
    using fun_print_org_t    = std::function<void(ORG&,std::ostream &)>;
    using fun_get_genome_t  = std::function<const genome_t & (ORG &)>;
    using fun_add_inject_t   = std::function<size_t(Ptr<ORG>)>;
    using fun_add_birth_t    = std::function<size_t(Ptr<ORG>, size_t)>;
    using fun_get_neighbor_t = std::function<size_t(size_t)>;

    // Internal state member variables
    Ptr<Random> random_ptr;         //< Random object to use.
    bool random_owner;              //< Did we create our own random number generator?
    emp::vector<Ptr<ORG>> pop;      //< All of the spots in the population.
    emp::vector<Ptr<ORG>> next_pop; //< Population being setup for next generation.
    size_t num_orgs;                //< How many organisms are actually in the population.
    emp::vector<double> fit_cache;  //< vector size == 0 when not caching; uncached values == 0.
    emp::vector<Ptr<genotype_t>> genotypes;      //< Genotypes for the corresponding orgs.
    emp::vector<Ptr<genotype_t>> next_genotypes; //< Genotypes for corresponding orgs in next_pop.

    // Configuration settings
    std::string name;         // Name of this world (for use in configuration.)
    bool cache_on;            // Should we be caching fitness values?
    size_t size_x;            // If a grid, track width; if pools, track pool size
    size_t size_y;            // If a grid, track height; if pools, track num pools.

    // Configurable functions.
    fun_calc_fitness_t fun_calc_fitness;  // Fitness function
    fun_do_mutations_t fun_do_mutations;  // Mutation function
    fun_print_org_t    fun_print_org;     // Print function
    fun_get_genome_t   fun_get_genome;    // Determine the genome object of an organism.
    fun_add_inject_t   fun_add_inject;    // Technique to inject a new organism.
    fun_add_birth_t    fun_add_birth;     // Technique to add a new offspring.
    fun_get_neighbor_t fun_get_neighbor;  // Choose a random neighbor near specified id.

    // Attributes are a dynamic way to track extra characteristics about a world.
    std::map<std::string, std::string> attributes;

    // Data collection.
    Systematics<genome_t> systematics;

    // AddOrgAt is the only way to add organisms (others must go through here)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype=nullptr);
    size_t AddNextOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype=nullptr);

    // RemoveOrgAt is the only way to remove organism.
    void RemoveOrgAt(size_t pos);
    void RemoveNextOrgAt(size_t pos);

    // Build a Setup function in world that calls ::Setup() on whatever is passed in IF it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);

    // Other private functions:
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }
    void ClearCache(size_t id) { if (id < fit_cache.size()) fit_cache[id] = 0.0; }

  public:
    World(Ptr<Random> rnd=nullptr, std::string _name="")
      : random_ptr(rnd), random_owner(false), pop(), next_pop(), num_orgs(0), fit_cache()
      , genotypes(), next_genotypes()
      , name(_name), cache_on(false), size_x(0), size_y(0)
      , fun_calc_fitness(), fun_do_mutations(), fun_print_org(), fun_get_genome()
      , fun_add_inject(), fun_add_birth(), fun_get_neighbor()
      , attributes(), systematics(true,true,true)
    {
      if (!rnd) NewRandom();
      SetDefaultFitFun<this_t, ORG>(*this);
      SetDefaultMutFun<this_t, ORG>(*this);
      SetDefaultPrintFun<this_t, ORG>(*this);
      SetDefaultGetGenomeFun<this_t, ORG>(*this);
      SetWellMixed();  // World default structure is well-mixed.
    }
    World(Random & rnd, std::string _name="") : World(&rnd, _name) { ; }
    World(std::string _name) : World(nullptr, _name) { ; }

    ~World() {
      Clear();
      if (random_owner) random_ptr.Delete();
    }

    // --- Publicly available types ---

    using org_t = ORG;
    using value_type = org_t;
    using iterator_t = World_iterator<this_t>;


    // --- Accessing Organisms or info ---

    size_t GetSize() const { return pop.size(); }
    size_t GetNumOrgs() const { return num_orgs; }
    bool IsOccupied(size_t i) const { return pop[i] != nullptr; }
    bool IsCacheOn() const { return cache_on; }
    size_t GetWidth() const { return size_x; }
    size_t GetHeight() const { return size_y; }

    // We ONLY have a const index operator since manipulations should go through other functions.
    // No non-const version.
    const ORG & operator[](size_t id) const {
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pop[id]);
    }
    const ORG & GetOrg(size_t id) const {
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pop[id]);
    }
    const ORG & GetOrg(size_t x, size_t y) { return GetOrg(x+y*size_x); }
    const Ptr<ORG> GetOrgPtr(size_t id) const {
      return pop[id];
    }

    const genome_t & GetGenome(ORG & org) { return fun_get_genome(org); }
    const genome_t & GetGenomeAt(size_t id) { return fun_get_genome(GetOrg(id)); }

    const Systematics<genome_t> & GetSystematics() const { return systematics; }
    void PrintLineage(size_t id, std::ostream & os=std::cout) const {
      systematics.PrintLineage(genotypes[id], os);
    }

    // --- CONFIGURE ---

    void SetWellMixed(bool synchronous_gen=false);
    void SetGrid(size_t width, size_t height, bool synchronous_gen=false);
    void SetPools(size_t num_pools, size_t pool_size, bool synchronous_gen=false);

    void SetFitFun(const fun_calc_fitness_t & fit_fun) { fun_calc_fitness = fit_fun; }
    void SetMutFun(const fun_do_mutations_t & mut_fun) { fun_do_mutations = mut_fun; }
    void SetPrintFun(const fun_print_org_t & print_fun) { fun_print_org = print_fun; }
    void SetGetGenomeFun(const fun_get_genome_t & gen_fun) { fun_get_genome = gen_fun; }


    // --- MANAGE ATTRIBUTES ---
    bool HasAttribute(const std::string & name) const { return Has(attributes, name); }
    const std::string & GetAttribute(const std::string) const {
      emp_assert( Has(attributes, name) );
      return Find(attributes, name, "UNKNOWN");
    }
    template <typename T>
    void SetAttribute(const std::string & name, T && val) { attributes[name] = to_string(val); }


    // --- UPDATE THE WORLD! ---

    void Update();
    template <typename... ARGS>
    void Execute(ARGS &&... args) {   // Redirect to all orgs in the population!
      for (Ptr<ORG> org : pop) { if (org) org->Execute(std::forward<ARGS>(args)...); }
    }


    // --- CALCULATE FITNESS ---

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

    // --- MUTATIONS! ---

    void DoMutations(ORG & org) {
      emp_assert(fun_do_mutations);
      emp_assert(random_ptr);
      fun_do_mutations(org, *random_ptr);
    }
    void DoMutationsID(size_t id) {
      emp_assert(pop[id]);
      DoMutations(*(pop[id]));
    }

    void MutatePop(size_t start_id=0) {
      for (size_t id = start_id; id < pop.size(); id++) { if (pop[id]) DoMutationsID(id); }
    }

    // --- MANIPULATE ORGS IN POPULATION ---

    void Clear();
    void ClearOrgAt(size_t pos);

    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pop.size(); i++) ClearOrgAt(i); // Remove orgs past new size.
      pop.resize(new_size, nullptr);                                // Default new orgs to null.
    }

    // Inject an organism using the default injection scheme.
    void Inject(const ORG & mem, size_t copy_count=1);

    // Inject an organism at a specific position.
    void InjectAt(const ORG & mem, const size_t pos);

    // Inject a random organism (constructor must facilitate!)
    template <typename... ARGS> void InjectRandomOrg(ARGS &&... args);

    // Place a newborn into the population, by default rules and with parent information.
    void DoBirth(const ORG mem, size_t parent_pos, size_t copy_count=1);

    // --- RANDOM FUNCTIONS ---

    Random & GetRandom() { return *random_ptr; }

    // Set or create a new random number generator.
    void SetRandom(Random & r);
    void NewRandom(int seed=-1);

    // Get any cell, at random.
    size_t GetRandomCellID() { return random_ptr->GetUInt(pop.size()); }

    // By default, assume a well-mixed population so random neighbors can be anyone.
    size_t GetRandomNeighborID(size_t id) { return fun_get_neighbor(id); }

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

    void Print(std::ostream & os = std::cout, const std::string & empty="X", const std::string & spacer=" ");
    void PrintOrgCounts(std::ostream & os = std::cout);
    void PrintGrid(std::ostream& os=std::cout, const std::string & empty="X", const std::string & spacer=" ");

    // --- FOR VECTOR COMPATIBILITY ---
    size_t size() const { return pop.size(); }
    void resize(size_t new_size) { Resize(new_size); }
    void clear() { Clear(); }

    iterator_t begin() { return iterator_t(this, 0); }
    iterator_t end() { return iterator_t(this, (int) pop.size()); }


    // --- SELECTION MECHANISMS ---
    // Elite Selection picks a set of the most fit individuals from the population to move to
    // the next generation.  Find top e_count individuals and make copy_count copies of each.
    void EliteSelect(size_t e_count=1, size_t copy_count=1);

    // Tournament Selection creates a tournament with a random sub-set of organisms,
    // finds the one with the highest fitness, and moves it to the next generation.
    // User provides the fitness function, the tournament size, and (optionally) the
    // number of tournaments to run.
    void TournamentSelect(size_t t_size, size_t tourny_count=1);
  };

  // =============================================================
  // ===                                                       ===
  // ===  Out-of-class member function definitions from above  ===
  // ===                                                       ===
  // =============================================================

  template <typename ORG>
  size_t World<ORG>::AddOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype) {
    emp_assert(new_org, pos);                            // The new organism must exist.

    // Determine new organism's genotype.
    Ptr<genotype_t> new_genotype = systematics.AddOrg(GetGenome(*new_org), p_genotype);
    if (pop.size() <= pos) pop.resize(pos+1, nullptr);               // Make sure we have room.
    if (pop[pos]) { ClearCache(pos); RemoveOrgAt(pos); --num_orgs; } // Clear out any old org.
    pop[pos] = new_org;                                              // Place new org.
    ++num_orgs;                                                      // Track number of orgs.

    // Track the new genotype.
    if (genotypes.size() <= pos) genotypes.resize(pos+1, nullptr);   // Make sure we fit genotypes.
    genotypes[pos] = new_genotype;

    return pos;
  }

  template <typename ORG>
  size_t World<ORG>::AddNextOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype) {
    emp_assert(new_org, pos);                            // The new organism must exist.

    // Determine new organism's genotype.
    Ptr<genotype_t> new_genotype = systematics.AddOrg(GetGenome(*new_org), p_genotype);
    if (next_pop.size() <= pos) next_pop.resize(pos+1, nullptr);   // Make sure we have room.
    if (next_pop[pos]) { RemoveNextOrgAt(pos); }                   // Clear out any old org.
    next_pop[pos] = new_org;                                       // Place new org.

    // Track the new genotype.
    if (next_genotypes.size() <= pos) next_genotypes.resize(pos+1, nullptr);  // Make sure we fit genotypes.
    next_genotypes[pos] = new_genotype;

    return pos;
  }

  template<typename ORG>
  void World<ORG>::RemoveOrgAt(size_t pos) {
    pop[pos].Delete();
    pop[pos] = nullptr;
    if (cache_on) {
      emp_assert(fit_cache.size() > pos);
      fit_cache[pos] = 0.0;
    }
    systematics.RemoveOrg( genotypes[pos] );
    genotypes[pos] = nullptr;
  }

  template<typename ORG>
  void World<ORG>::RemoveNextOrgAt(size_t pos) {
    next_pop[pos].Delete();
    next_pop[pos] = nullptr;
    systematics.RemoveOrg( next_genotypes[pos] );
    next_genotypes[pos] = nullptr;
  }

  template<typename ORG>
  void World<ORG>::SetWellMixed(bool synchronous_gen) {
    size_x = 0; size_y = 0;

    // -- Setup functions --
    // Append at end of population
    fun_add_inject = [this](Ptr<ORG> new_org) {
      return AddOrgAt(new_org, pop.size());
    };

    // neighbors are anywhere in the population.
    fun_get_neighbor = [this](size_t) { return GetRandomCellID(); };

    if (synchronous_gen) {
      // Append births into the next population.
      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
        emp_assert(new_org);                            // New organism must exist.
        return AddNextOrgAt(new_org, next_pop.size(), genotypes[parent_id]);  // Append it to the NEXT population
      };

      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neigbor in current population.
      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
        return AddOrgAt(new_org, fun_get_neighbor(parent_id), genotypes[parent_id]); // Place org in existing population.
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Mixed");
  }

  template<typename ORG>
  void World<ORG>::SetGrid(size_t width, size_t height, bool synchronous_gen) {
    size_x = width;  size_y = height;
    Resize(size_x * size_y);

    // -- Setup functions --
    // Inject a random position in grid
    fun_add_inject = [this](Ptr<ORG> new_org) {
      return AddOrgAt(new_org, GetRandomCellID());
    };

    // neighbors are in 9-sized neighborhood.
    fun_get_neighbor = [this](size_t id) {
      emp_assert(random_ptr);
      const int offset = random_ptr->GetInt(9);
      const int rand_x = (int) (id%size_x) + offset%3 - 1;
      const int rand_y = (int) (id/size_x) + offset/3 - 1;
      return (size_t) (emp::Mod(rand_x, (int) size_x) + emp::Mod(rand_y, (int) size_y) * (int)size_x);
    };

    if (synchronous_gen) {
      // Place births in a neighboring position in the new grid.
      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
        emp_assert(new_org);                                  // New organism must exist.
        const size_t id = fun_get_neighbor(parent_id);     // Placed near parent, in next pop.
        return AddNextOrgAt(new_org, id, genotypes[parent_id]);
      };
      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neigbor in current population.
      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
        return AddOrgAt(new_org, fun_get_neighbor(parent_id), genotypes[parent_id]); // Place org in existing population.
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Grid");
  }

  template<typename ORG>
  void World<ORG>::SetPools(size_t num_pools, size_t pool_size, bool synchronous_gen) {
    size_x = pool_size;  size_y = num_pools;
    Resize(size_x * size_y);

    // -- Setup functions --
    // Inject in a empty pool -or- randomly if none empty
    fun_add_inject = [this](Ptr<ORG> new_org) {
      for (size_t id = 0; id < pop.size(); id += size_x) {
        if (pop[id] == nullptr) return AddOrgAt(new_org, id);
      }
      return AddOrgAt(new_org, GetRandomCellID());
    };

    // neighbors are everyone in the same pool.
    fun_get_neighbor = [this](size_t id) {
      emp_assert(random_ptr);
      return (id / size_x) * size_x + random_ptr->GetUInt(size_x);
    };

    if (synchronous_gen) {
      // Place births in the next open spot in the new pool (or randomly if full!)
      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
        emp_assert(new_org);                                  // New organism must exist.
        const size_t pool_id = parent_id / size_x;
        const size_t start_id = pool_id * size_x;
        for (size_t id = start_id; id < start_id+size_x; id++) {
          if (next_pop[id] == nullptr) {  // Search for an open positions...
            return AddNextOrgAt(new_org, id, genotypes[parent_id]);
          }
        }
        const size_t id = fun_get_neighbor(parent_id);     // Placed near parent, in next pop.
        return AddNextOrgAt(new_org, id, genotypes[parent_id]);
      };
      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neigbor in current population.
      fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
        return AddOrgAt(new_org, fun_get_neighbor(parent_id), genotypes[parent_id]); // Place org in existing population.
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Pools");
  }

  // --- Updating the world! ---

  template<typename ORG>
  void World<ORG>::Update() {
    // If generations are synchronous (i.e, next_pop is not empty), put the next generation in place.
    if (next_pop.size()) {
      // Clear out current pop.
      for (size_t i = 0; i < pop.size(); i++) if (pop[i]) RemoveOrgAt(i);
      pop.resize(0);

      std::swap(pop, next_pop);               // Move next_pop into place.
      std::swap(genotypes, next_genotypes);   // Move next_genotypes into place.

      num_orgs = 0;             // Update the organism count.
      for (size_t i = 0; i < pop.size(); i++) if (pop[i]) ++num_orgs;
    }
  }

  template<typename ORG>
  double World<ORG>::CalcFitnessID(size_t id) {
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
  template<typename ORG>
  void World<ORG>::Clear() {
    for (size_t i = 0; i < pop.size(); i++) if (pop[i]) RemoveOrgAt(i);
    for (size_t i = 0; i < next_pop.size(); i++) if (next_pop[i]) RemoveNextOrgAt(i);
    pop.resize(0);                                     // Remove deleted organisms.
    next_pop.resize(0);
    fit_cache.resize(0);
    num_orgs = 0;
  }

  // Delete organism at a specified position.
  template<typename ORG>
  void World<ORG>::ClearOrgAt(size_t pos) {
    if (!pop[pos]) return;  // No organism; no need to do anything.
    RemoveOrgAt(pos);
    pop[pos]=nullptr;
    ClearCache(pos);
    num_orgs--;
  }

  template <typename ORG>
  void World<ORG>::Inject(const ORG & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      //const size_t pos =
      fun_add_inject(new_org);
      //SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template <typename ORG>
  void World<ORG>::InjectAt(const ORG & mem, const size_t pos) {
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    AddOrgAt(new_org, pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG>
  template <typename... ARGS>
  void World<ORG>::InjectRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InjectRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    // const size_t pos =
    fun_add_inject(new_org);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG>
  void World<ORG>::DoBirth(const ORG mem, size_t parent_pos, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      // const size_t pos =
      fun_add_birth(new_org, parent_pos);
      // SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template<typename ORG>
  void World<ORG>::SetRandom(Random & r) {
    if (random_owner) random_ptr.Delete();
    random_ptr = &r;
    random_owner = false;
  }

  template<typename ORG>
  void World<ORG>::NewRandom(int seed) {
    if (random_owner) random_ptr.Delete();
    random_ptr.New(seed);
    random_owner = true;
  }

  // Get random *occupied* cell.
  template<typename ORG>
  size_t World<ORG>::GetRandomOrgID() {
    emp_assert(num_orgs > 0); // Make sure it's possible to find an organism!
    size_t pos = random_ptr->GetUInt(0, pop.size());
    while (pop[pos] == nullptr) pos = random_ptr->GetUInt(0, pop.size());
    return pos;
  }

  // Find ALL cell IDs the return true in the filter.
  template<typename ORG>
  emp::vector<size_t> World<ORG>::FindCellIDs(const std::function<bool(ORG*)> & filter) {
    emp::vector<size_t> valid_IDs(0);
    for (size_t i = 0; i < pop.size(); i++) {
      if (filter(pop[i].Raw())) valid_IDs.push_back(i);
    }
    return valid_IDs;
  }

  // Run population through a bottleneck to (potentially) shrink it.
  template<typename ORG>
  void World<ORG>::DoBottleneck(const size_t new_size, bool choose_random) {
    if (new_size >= pop.size()) return;  // No bottleneck needed!

    // If we are supposed to keep only random organisms, shuffle the beginning into place!
    if (choose_random) emp::Shuffle<Ptr<ORG>>(*random_ptr, pop, new_size);

    // Clear out all of the organisms we are removing and resize the population.
    for (size_t i = new_size; i < pop.size(); ++i) ClearOrgAt(i);
    pop.resize(new_size);
  }

  template<typename ORG>
  void World<ORG>::Print(std::ostream & os, const std::string & empty, const std::string & spacer) {
    for (Ptr<ORG> org : pop) {
      if (org) os << fun_print_org(*org, os);
      else os << empty;
      os << spacer;
    }
  }

  template<typename ORG>
  void World<ORG>::PrintOrgCounts(std::ostream & os) {
    std::map<ORG,size_t> org_counts;
    for (Ptr<ORG> org : pop) if (org) org_counts[*org] = 0;  // Initialize needed entries
    for (Ptr<ORG> org : pop) if (org) org_counts[*org] += 1; // Count actual types.
    for (auto x : org_counts) {
      ORG cur_org = x.first;
      fun_print_org(cur_org, os);
      os << " : " << x.second << std::endl;
    }
  }

  template<typename ORG>
  void World<ORG>::PrintGrid(std::ostream& os,
                                      const std::string & empty, const std::string & spacer) {
    for (size_t y=0; y < size_y; y++) {
      for (size_t x = 0; x < size_x; x++) {
        Ptr<ORG> org = GetOrgPtr(x+y*size_x);
        if (org) fun_print_org(*org, os);
        else os << empty;
        os << spacer;
      }
      os << std::endl;
    }
  }

  // Elite Selection picks a set of the most fit individuals from the population to move to
  // the next generation.  Find top e_count individuals and make copy_count copies of each.
  template<typename ORG>
  void World<ORG>::EliteSelect(size_t e_count, size_t copy_count) {
    emp_assert(fun_calc_fitness);
    emp_assert(e_count > 0 && e_count <= pop.size(), e_count);
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
      DoBirth( *(pop[m->second]), m->second, copy_count);
      ++m;
    }
  }

  // Tournament Selection creates a tournament with a random sub-set of organisms,
  // finds the one with the highest fitness, and moves it to the next generation.
  // User provides the fitness function, the tournament size, and (optionally) the
  // number of tournaments to run.
  template<typename ORG>
  void World<ORG>::TournamentSelect(size_t t_size, size_t tourny_count) {
    emp_assert(fun_calc_fitness);
    emp_assert(t_size > 0 && t_size <= num_orgs, t_size, num_orgs);
    emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");

    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      // Choose organisms for this tournament (with replacement!)
      for (size_t i=0; i < t_size; i++) entries.push_back( GetRandomOrgID() );

      double best_fit = CalcFitnessID(entries[0]);
      size_t best_id = entries[0];

      // Search for a higher fit org in the tournament.
      for (size_t i = 1; i < t_size; i++) {
        const double cur_fit = CalcFitnessID(entries[i]);
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          best_id = entries[i];
        }
      }

      // Place the highest fitness into the next generation!
      DoBirth( *(pop[best_id]), best_id, 1 );
    }
  }


}

#endif
