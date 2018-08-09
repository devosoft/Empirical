/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  World.h
 *  @brief A world management class, putting MABE modles together.
 *
 *  A world assembles a set of MABE modules into an evolving population.  It also
 *  automatically sets up a configurtion system and uses reasonable default linkages
 *  (that can be easily overridden).
 */

#ifndef MABE_WORLD_H
#define MABE_WORLD_H

#include <iostream>
#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "config/ArgManager.h"
#include "config/config.h"
#include "control/Signal.h"
#include "control/SignalControl.h"
#include "data/DataFile.h"
#include "meta/TypePack.h"
#include "tools/Random.h"
#include "tools/tuple_utils.h"

#include "types.h"

#include "World_structure.h"

namespace mabe {

  class World {
  public:
    using value_type = OrganismBase;   // For compatability with vectors.
    using org_t = OrganismBase;
    using org_ptr_t = emp::Ptr<org_t>;
    using pop_t = emp::vector<org_ptr_t>;

  private:
    /// Function type for calculating fitness, typically set by the environment.
    using fun_calc_fitness_t    = std::function<double(OrganismBase&)>;

    using environments_t = emp::vector<emp::Ptr<EnvironmentBase>>;    ///< Type of vector of all Environments
    using organism_types_t = emp::vector<emp::Ptr<OrganismTypeBase>>; ///< Type of vector of all OrganismTypes
    using schemas_t = emp::vector<emp::Ptr<SchemaBase>>;              ///< Type of vector of all Schemas

    /// Type of tuple of all modules to be built in the world.
    using modules_t = std::tuple<environments_t, organism_types_t, schemas_t>;

    /// Type of master World config file.
    EMP_BUILD_CONFIG( config_t,
      GROUP(DEFAULT_GROUP, "Master World Settings"),
      VALUE(RANDOM_SEED, int, 0, "Seed for main random number generator. Use 0 for based on time."),
    )  

    // ----- World MODULES -----

    modules_t modules;    ///< Pointers to all modules, divided into tuple of type-specific vectors.

    environments_t & environments;     ///< Direct link to environments vector of modules. 
    organism_types_t & organism_types; ///< Direct link to organism types vector of modules. 
    schemas_t & schemas;               ///< Direct link to schemas vector of modules. 

    config_t config;                   ///< Master configuration object.

    void AddModule(emp::Ptr<EnvironmentBase> env_ptr) { environments.push_back(env_ptr); }
    void AddModule(emp::Ptr<OrganismTypeBase> pop_ptr) { organism_types.push_back(pop_ptr); }
    void AddModule(emp::Ptr<SchemaBase> schema_ptr) { schemas.push_back(schema_ptr); }

    void ForEachModule(std::function<void(emp::Ptr<ModuleBase>)> fun) {
      for (emp::Ptr<ModuleBase> x : environments)    { fun(x); }
      for (emp::Ptr<ModuleBase> x : organism_types)  { fun(x); }
      for (emp::Ptr<ModuleBase> x : schemas)         { fun(x); }
    }


    // ----- World STATE -----
    const std::string name;           ///< Unique name for this World (for use in configuration.)
    size_t update;                    ///< How many times has Update() been called?
    emp::Random random;               ///< Random object to use.
    WorldVector pops;                 ///< Set of active [0] and "next" [1] orgs in population.
    pop_t & active_pop;               ///< Shortcut to pops[0]
    pop_t & next_pop;                 ///< Shortcut to pops[1]
    size_t num_orgs;                  ///< How many organisms are actually in the population.
    emp::vector<double> fit_cache;    ///< vector size==0 when not caching; uncached values==0.0


    // ----- World CONFIG ----
    bool cache_on;                  ///< Should we be caching fitness values?
    std::vector<size_t> pop_sizes;  ///< Sizes of population dimensions (eg, 2 vals for grid)
    emp::TraitSet<OrganismBase> phenotypes;  ///< What phenotypes are we tracking?
    emp::vector<emp::Ptr<emp::DataFile>> files;    ///< Output files.

    bool is_synchronous;            ///< Does this world have synchronous generations?
    bool is_space_structured;       ///< Do we have a spatially structured population?
    bool is_pheno_structured;       ///< Do we have a phenotypically structured population?

    fun_calc_fitness_t     fun_calc_fitness;    ///< ...evaluate fitness for provided organism.    
 
    /// Attributes are a dynamic way to track extra characteristics about a world.
    std::map<std::string, std::string> attributes;

    // @CAO: Still need to port over systematics!

     // == Signals ==
    emp::SignalControl control;  // Setup the world to control various signals.
    
    /// Trigger signal... before organism gives birth w/parent position.
    emp::Signal<void(size_t)> before_repro_sig;
    
    /// Trigger signal... when offspring organism is built.
    emp::Signal<void(org_ptr_t,size_t)> offspring_ready_sig;
    
    /// Trigger signal... when outside organism is ready to inject.
    emp::Signal<void(org_ptr_t)> inject_ready_sig;
    
    /// Trigger signal... before placing any organism into target cell.
    emp::Signal<void(org_ptr_t,size_t)> before_placement_sig;
    
    /// Trigger signal... after any organism is placed into world.
    emp::Signal<void(size_t)> on_placement_sig;
    
    /// Trigger signal... at the beginning of Update()
    emp::Signal<void(size_t)> on_update_sig;
    
    /// Trigger signal... immediately before any organism dies.
    emp::Signal<void(size_t)> on_death_sig;
    
    /// Trigger signal... after org positions are swapped
    emp::Signal<void(WorldPosition,WorldPosition)> on_swap_sig;
    
    /// Trigger signal... in the World destructor.
    emp::Signal<void()> world_destruct_sig;
    
  public:
    World(const std::string & _name="World")
      : environments(std::get<environments_t>(modules))
      , organism_types(std::get<organism_types_t>(modules))
      , schemas(std::get<schemas_t>(modules))
      , name(_name), update(0), random()
      , pops(), active_pop(pops[0]), next_pop(pops[1]), num_orgs(0)
      , fit_cache(), cache_on(false), pop_sizes(1,0), phenotypes(), files()
      , is_synchronous(false), is_space_structured(false), is_pheno_structured(false)
      , fun_calc_fitness()
      , attributes(), control()
      , before_repro_sig(emp::to_string(name,"::before-repro"), control)
      , offspring_ready_sig(emp::to_string(name,"::offspring-ready"), control)
      , inject_ready_sig(emp::to_string(name,"::inject-ready"), control)
      , before_placement_sig(emp::to_string(name,"::before-placement"), control)
      , on_placement_sig(emp::to_string(name,"::on-placement"), control)
      , on_update_sig(emp::to_string(name,"::on-update"), control)
      , on_death_sig(emp::to_string(name,"::on-death"), control)
      , on_swap_sig(emp::to_string(name,"::on-swap"), control)
      , world_destruct_sig(emp::to_string(name,"::wolrd-destruct"), control)
    {
    }

    ~World() {
      Clear();
      world_destruct_sig.Trigger();
      ForEachModule( [](emp::Ptr<ModuleBase> x){ x.Delete(); } );
      for (auto file : files) file.Delete();
    }

    /// How many organisms can fit in the world?
    size_t GetSize() const { return pops[0].size(); }

    /// How many organisms are currently in the world?
    size_t GetNumOrgs() const { return num_orgs; }

    /// What update number is the world currently on? (assumes Update() is being used)
    size_t GetUpdate() const { return update; }

    /// How many cells wide is the world? (assumes grids are active.)
    size_t GetWidth() const { return pop_sizes[0]; }

    /// How many cells tall is the world? (assumes grids are active.)
    size_t GetHeight() const { return pop_sizes[1]; }

    /// Get the full population to analyze externally.
    const pop_t & GetFullPop() const { return active_pop; }

    /// What phenotypic traits is the population tracking?
    const emp::TraitSet<org_t> & GetPhenotypes() const { return phenotypes; }

    /// Add an already-constructed datafile.
    emp::DataFile & AddDataFile(emp::Ptr<emp::DataFile> file);

    /// Lookup a file by name.
    emp::DataFile & GetFile(const std::string & filename) {
      for (emp::Ptr<emp::DataFile> file : files) {
        if (file->GetFilename() == filename) return *file;
      }
      emp_assert(false, "Trying to lookup a file that does not exist.", filename);
      return *(files[0]);
    }

    /// Does the specified cell ID have an organism in it?
    bool IsOccupied(WorldPosition pos) const { return pops.IsValid(pos) && pops(pos); }

    /// Are we currently caching fitness values?
    bool IsCacheOn() const { return cache_on; }

    /// Are generations being evaluated synchronously?
    /// (i.e., Update() places all births into the population after removing all current organisms)
    bool IsSynchronous() const { return is_synchronous; }

    /// Is there some sort of spatial structure to the population?
    /// (i.e., are some organisms closer together than others.)
    bool IsSpaceStructured() const { return is_space_structured; }

    /// Is there some sort of structure to the population based on phenotype?
    /// (i.e., are phenotypically-similar organisms forced to be closer together?)
    bool IsPhenoStructured() const { return is_pheno_structured; }

    /// Denote that this World will be treated as having synchronous generations.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    World & MarkSynchronous(bool in=true) { is_synchronous = in; return *this; }

    /// Denote that the World will have a spatial structure to the organisms.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    World & MarkSpaceStructured(bool in=true) { is_space_structured = in; return *this; }

    /// Denote that the World will have organisms structured based on phenotype.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    World & MarkPhenoStructured(bool in=true) { is_pheno_structured = in; return *this; }

    /// Index into a world to obtain a const reference to an organism.  Any manipulations to
    /// organisms should go through other functions to be tracked appropriately.
    /// Will trip assert if cell is not occupied.
    const org_t & operator[](size_t id) const {
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[0][id]);
    }

    /// Retrieve a reference to the organsim as the specified position.
    /// Same as operator[]; will trip assert if cell is not occupied.
    org_t & GetOrg(size_t id) {
      emp_assert(id < pop.size());         // Pop must be large enough.
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[0][id]);
    }

    /// Retrieve a const reference to the organsim as the specified x,y coordinates.
    /// @CAO: Technically, we should set this up with any number of coordinates.
    org_t & GetOrg(size_t x, size_t y) { return GetOrg(x+y*GetWidth()); }

    /// Retrive a pointer to the contents of a speciefied cell; will be nullptr if the cell is
    /// not occupied.
    const org_ptr_t GetOrgPtr(size_t id) const { return pops[0][id]; }

    /// Retrieve a reference to the organsim as the specified position in the NEXT population.
    /// Will trip assert if cell is not occupied.
    org_t & GetNextOrg(size_t id) {
      emp_assert(id < pops[1].size());         // Next pop must be large enough.
      emp_assert(pops[1][id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[1][id]);
    }



    /// Build a new module in the World.    
    template <typename T>
    T & BuildModule(const std::string name) {
      using module_t = to_module_t<T>;                   // Determine category type of module.
      using mvector_t = emp::vector<emp::Ptr<module_t>>; // Determine vector type for category.
      emp::Ptr<T> new_mod = emp::NewPtr<T>(name);        // Build the new module.
      std::get<mvector_t>(modules).push_back(new_mod);   // Add new module to appropriate vector.
      config.AddNameSpace(new_mod->GetConfig(), name);   // Setup this module's config in a namespace.
      return *new_mod;                                   // Return the final module.
    }

    bool Config(int argc, char * argv[], const std::string & filename,
                const std::string & macro_filename="") {
      config.Read(filename, false);
      auto args = emp::cl::ArgManager(argc, argv);
      bool config_ok = args.ProcessConfigOptions(config, std::cout, filename, macro_filename);
      if (!config_ok || args.HasUnknown()) return false;  // If there are leftover args, fail!

      // Setup World with Config options.
      random.ResetSeed(config.RANDOM_SEED());

      // Now that all of the modules have been configured, allow them to setup the world.
      ForEachModule( [this](emp::Ptr<ModuleBase> x){ x->Setup(*this); } );

      return true;
    }
    
    int Run() {
      return 0;
    }

    void Clear() {
      for (size_t pop_id = 0; pop_id < 2; pop_id++) {
        for (size_t i = 0; i < pops[pop_id].size(); i++) RemoveOrgAt(WorldPosition(i,pop_id));
        pops[pop_id].resize(0);
      }
    }

    /// Clear all of the orgs and reset stats.
    void Reset() { Clear(); update = 0; }

    /// Swap the positions of two organisms.
    void Swap(WorldPosition pos1, WorldPosition pos2) {
      std::swap(pops(pos1), pops(pos2));
      on_swap_sig.Trigger(pos1, pos2);
    }

    /// Change the size of the world.  If the new size is smaller than the old, remove any
    /// organisms outside the new range.  If larger, new positions are empty.
    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pops[0].size(); i++) RemoveOrgAt(i); // Remove orgs past new size.
      pops[0].resize(new_size, nullptr);                                 // Default new orgs to null.
    }

    /// Change the size of the world based on width and height.
    void Resize(size_t new_width, size_t new_height) {
      Resize(new_width * new_height);
      pop_sizes.resize(2);
      pop_sizes[0] = new_width; pop_sizes[1] = new_height;
    }

    /// Change the size of the world based on a vector of dimensions.
    void Resize(const emp::vector<size_t> & dims) {
      Resize(emp::Product(dims));
      pop_sizes = dims;
    }

    /// AddOrgAt is the core function to add organisms to the population (others must go through here)
    /// Note: This function ignores population structure, so requires you to manage your own structure.
    void AddOrgAt(org_ptr_t new_org, WorldPosition pos, WorldPosition p_pos=WorldPosition());

    /// RemoveOrgAt is the core function to remove organisms from the population.
    /// Note: This function ignores population structure, so requires you to manage your own structure.
    void RemoveOrgAt(WorldPosition pos);


    void PrintStatus() {
      std::cout << "Environemnts: " << environments.size() << std::endl;
      for (auto x : environments) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
      std::cout << "Organism Types: " << organism_types.size() << std::endl;
      for (auto x : organism_types) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
      std::cout << "Schemas: " << schemas.size() << std::endl;
      for (auto x : schemas) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
    }
  };

  // =============================================================
  // ===                                                       ===
  // ===  Out-of-class member function definitions from above  ===
  // ===                                                       ===
  // =============================================================

  void World::AddOrgAt(emp::Ptr<OrganismBase> new_org, WorldPosition pos, WorldPosition p_pos) {
    emp_assert(new_org);         // The new organism must exist.
    emp_assert(pos.IsValid());   // Position must be legal.

    // If new organism is going into the active population, trigger signal before doing so.
    if (pos.IsActive()) { before_placement_sig.Trigger(new_org, pos.GetIndex()); }

    // for (Ptr<SystematicsBase<ORG> > s : systematics) {
    //   s->SetNextParent((int) p_pos.GetIndex());
    // }

    // Clear out any old organism at this position.
    RemoveOrgAt(pos);

    pops.MakeValid(pos);                 // Make sure we have room for new organism
    pops(pos) = new_org;                 // Put org into place.

    // Track org count
    if (pos.IsActive()) ++num_orgs;

    // Track the new systematics info
    // for (Ptr<SystematicsBase<ORG> > s : systematics) {
    //   s->AddOrg(*new_org, (int) pos.GetIndex(), (int) update, !pos.IsActive());
    // }

    // new_org->Setup(pos, random);

    // If new organism is in the active population, trigger associated signal.
    if (pos.IsActive()) { on_placement_sig.Trigger(pos.GetIndex()); }
  }

  void World::RemoveOrgAt(WorldPosition pos) {
    size_t id = pos.GetIndex();                       // Identify specific index.
    pop_t & cur_pop = pops[pos.GetPopID()];
    if (id >= cur_pop.size() || !cur_pop[id]) return; // Nothing to remove!
    if (pos.IsActive()) on_death_sig.Trigger(id);    // If active, signal that org is about to die.
    cur_pop[id].Delete();                            // Delete the organism...
    cur_pop[id] = nullptr;                           // ...and reset the pointer to null

    if (pos.IsActive()) {
      --num_orgs;                                    // Track one fewer organisms in the population
      // if (cache_on) ClearCache(id);                  // Delete any cached info about this organism
      // for (Ptr<SystematicsBase<ORG> > s : systematics) {
      //   s->RemoveOrg((int) pos.GetIndex());          // Notify systematics about organism removal
      // }
    } else {
      // for (Ptr<SystematicsBase<ORG> > s : systematics) {
      //   s->RemoveNextOrg((int) pos.GetIndex());      // Notify systematics about organism removal
      // }
    }
  }
}

#endif

