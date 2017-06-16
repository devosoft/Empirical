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
    using fun_add_inject_t   = std::function<size_t(Ptr<ORG>)>;
    using fun_add_birth_t    = std::function<size_t(Ptr<ORG>, size_t)>;

    // Enumerated values
    enum class Struct { MIXED, GRID, POOLS, EXTERNAL };

    // Internal state member variables
    Ptr<Random> random_ptr;         // Random object to use.
    bool random_owner;              // Did we create our own random number generator?
    pop_t pop;                      // All of the spots in the population.
    pop_t next_pop;                 // Population being setup for next generation.
    size_t num_orgs;                // How many organisms are actually in the population.
    emp::vector<double> fit_cache;  // vector size == 0 when not caching; uncached values == 0.

    // Configuration settings
    Struct pop_struct;        // What population structure are we using?
    bool synchronous_gen;     // Should generations be prefectly synchronous?
    bool cache_on;            // Should we be caching fitness values?
    size_t width;             // If a grid, track size.
    size_t height;

    // Configurable functions.
    fun_calc_fitness_t fun_calc_fitness;  // Fitness function
    fun_add_inject_t   fun_add_inject;    // Technique to inject a new organism.
    fun_add_birth_t    fun_add_birth;     // Technique to add a new offspring.


    // AddOrgAt is the only way to add organisms (others must go through here)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos);

    // Build a Setup function in world that calls ::Setup() on whatever is passed in IF it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);

    // Other private functions:
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }

  public:
    World(Ptr<Random> rnd=nullptr)
      : random_ptr(rnd), random_owner(false), pop(), num_orgs(0)
      , pop_struct(Struct::MIXED), synchronous_gen(false), cache_on(false), width(0), height(0)
      , fun_calc_fitness(), fun_add_inject(), fun_add_birth()
    {
      if (!rnd) NewRandom();
      ConfigFuns();
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
    bool IsSynchronous() const { return synchronous_gen; }
    bool IsCacheOn() const { return cache_on; }
    Struct GetPopStruct() const { return pop_struct; }
    size_t GetWidth() const { return width; }
    size_t GetHeight() const { return height; }

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
    const ORG & GetOrg(size_t x, size_t y) { return GetOrg(x+y*width); }


    // --- CONFIGURE ---

    void ConfigFuns();
    void ModeBasic() { synchronous_gen = false; ConfigFuns(); }
    void ModeEA() { synchronous_gen = true; ConfigFuns(); }
    void SetWellMixed() {
      width = 0; height = 0;
      pop_struct(Struct::MIXED);
      ConfigFuns();
    }
    void SetGrid(size_t _w, size_t _h) {
      width = _w;  height = _h;
      pop_struct(Struct::GRID);
      ConfigFuns();
    }


    // --- UPDATE THE WORLD! ---

    void Update();
    template <typename... ARGS>
    void Execute(ARGS &&... args) {   // Redirect to all orgs in the population!
      for (Ptr<ORG> org : pop) { if (org) org->Execute(std::forward<ARGS>(args)...); }
    }


    // --- CALCULATE FITNESS ---

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
    size_t GetRandomCellID() { return random_ptr->GetInt(0, pop.size()); }

    // By default, assume a well-mixed population so random neighbors can be anyone.
    size_t GetRandomNeighborID(size_t id) {
      // @CAO: Change to a member function pointer?
      switch (pop_struct) {
      case Struct::MIXED: return random_ptr->GetUInt(0, pop.size());
      case Struct::GRID: {
        const int offset = random_ptr->GetInt(9);
        const int rand_x = (int) (id%width) + offset%3 - 1;
        const int rand_y = (int) (id/width) + offset/3 - 1;
        return (size_t) (emp::Mod(rand_x, (int) width) + emp::Mod(rand_y, (int) height) * (int)width);
      }
      case Struct::POOLS:
      case Struct::EXTERNAL:
        break;
      }
      emp_assert(false);
      return 0;
    }

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
    void PrintGrid(std::function<std::string(ORG*)> string_fun, std::ostream& os=std::cout,
               const std::string & empty="-", const std::string & spacer=" ") {
      emp_assert(string_fun);
      for (size_t y=0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
          ptr_t org = GetOrg(x+y*width);
          if (org) os << string_fun(org.Raw()) << spacer;
          else os << empty << spacer;
        }
        os << std::endl;
      }
    }
    void PrintGrid(std::ostream& os=std::cout, const std::string & empty="X", std::string spacer=" ") {
      PrintGrid( [](ORG * org){return emp::to_string(*org);}, os, empty, spacer);
    }

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

  template<typename ORG, typename GENOTYPE>
  size_t World<ORG,GENOTYPE>::AddOrgAt(Ptr<ORG> new_org, size_t pos) {
    emp_assert(new_org, pos);

    if (pop.size() <= pos) pop.resize(pos+1, nullptr);   // Make sure we have room.
    if (pop[pos]) { pop[pos].Delete(); --num_orgs; }     // Clear out any old org.
    pop[pos] = new_org;                                  // Place new org.
    ++num_orgs;                                          // Track number of orgs.

    return pos;
  }

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::ConfigFuns() {
    // Setup AddInject...
    switch (pop_struct) {
    case Struct::MIXED:
      fun_add_inject = [this](Ptr<ORG> new_org) { return AddOrgAt(new_org, pop.size()); };
      break;
    case Struct::GRID:
      break;
    case Struct::POOLS:
      break;
    case Struct::EXTERNAL:
      // Do nothing; these should be set... externally.
      break;
    }

    // Setup AddBirth, which may be based on population structure.
    if (synchronous_gen) {
    switch (pop_struct) {
      case Struct::MIXED:
        fun_add_birth = [this](Ptr<ORG> new_org, size_t) {
          emp_assert(new_org);                           // New organism must exist.
          next_pop.push_back(new_org);
          return next_pop.size() - 1;
        };
        break;
      case Struct::GRID:
        fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
          emp_assert(new_org);                           // New organism must exist.
          const size_t id = GetRandomNeighborID(parent_id);
          if (id >= next_pop.size()) next_pop.resize(id+1, nullptr);
          if (next_pop[id]) next_pop[id].Delete();
          next_pop[id] = new_org;
          return id;
        };
        break;
      case Struct::POOLS:
        break;
      case Struct::EXTERNAL:
        // Do nothing; these should be set... externally.
        break;
      }
    }

    // Otherwise asynchronous...
    else {
      switch (pop_struct) {
      case Struct::MIXED:
      case Struct::GRID:
        fun_add_inject = [this](Ptr<ORG> new_org) { return AddOrgAt(new_org, pop.size()); };
        fun_add_birth = [this](Ptr<ORG> new_org, size_t parent_id) {
          emp_assert(new_org);                          // New organism must exist.
          size_t id = GetRandomNeighborID(parent_id);   // Place in random position.
          return AddOrgAt(new_org, id);                 // Place org in  existing population.
        };
        break;
      case Struct::POOLS:
        break;
      case Struct::EXTERNAL:
        // Do nothing; these should be set... externally.
        break;
      }
    }

  }


  // --- Updating the world! ---

  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::Update() {
    std::cout << ":: pop.size() = " << pop.size() << std::endl;
    std::cout << ":: next_pop.size() = " << next_pop.size() << std::endl;

    // If generations are synchronous, put the next generation in place.
    if (synchronous_gen) {
      // Add all waiting organisms into the population.
      for (size_t i = 0; i < next_pop.size(); i++) {
        if (next_pop[i]) AddOrgAt(next_pop[i], i);
      }
      // Remove any remaining organisms in main population.
      for (size_t i = next_pop.size(); i < pop.size(); i++) {
        if (pop[i]) pop[i].Delete();
      }
      // Reset populations.
      pop.resize(next_pop.size());
      next_pop.resize(0);
    }
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
    for (ptr_t org : pop) if (org) org.Delete();       // Delete current organisms.
    for (ptr_t org : next_pop) if (org) org.Delete();  // Delete waiting organisms.
    pop.resize(0);                                     // Remove deleted organisms.
    next_pop.resize(0);
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
  void World<ORG,GENOTYPE>::Inject(const ORG & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      //const size_t pos =
      fun_add_inject(new_org);
      //SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template <typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::InjectAt(const ORG & mem, const size_t pos) {
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    AddOrgAt(new_org, pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG, typename GENOTYPE>
  template <typename... ARGS>
  void World<ORG,GENOTYPE>::InjectRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InjectRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    // const size_t pos =
    fun_add_inject(new_org);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::DoBirth(const ORG mem, size_t parent_pos, size_t copy_count) {
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

  // Elite Selection picks a set of the most fit individuals from the population to move to
  // the next generation.  Find top e_count individuals and make copy_count copies of each.
  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::EliteSelect(size_t e_count, size_t copy_count) {
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
  template<typename ORG, typename GENOTYPE>
  void World<ORG,GENOTYPE>::TournamentSelect(size_t t_size, size_t tourny_count) {
    emp_assert(fun_calc_fitness);
    emp_assert(t_size > 0 && t_size <= num_orgs, t_size, num_orgs);
    emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");

    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      // Choose organisms for this tournament (with replacement!)
      for (size_t i=0; i<t_size; i++) entries.push_back( GetRandomOrgID() );

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
