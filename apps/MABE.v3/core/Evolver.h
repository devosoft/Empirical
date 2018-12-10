/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Evolver.h
 *  @brief A MABE module management class.
 *
 *  An Evolver is a high-level MABE controller that assembles a set of modules into an evolving
 *  population.  It automatically sets up a configurtion system and uses intuitive default
 *  linkages (that can be easily overridden).
 * 
 * 
 *  Developer Notes:
 *  - Merge attributes into DataMap
 */

#ifndef MABE_EVOLVER_H
#define MABE_EVOLVER_H

#include <iostream>
#include <string>

#include "base/array.h"
#include "base/assert.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "config/ArgManager.h"
#include "config/config.h"
#include "control/Signal.h"
#include "control/SignalControl.h"
#include "data/DataFile.h"
#include "data/Trait.h"
#include "meta/TypePack.h"
#include "tools/math.h"
#include "tools/Random.h"
#include "tools/tuple_utils.h"
#include "tools/vector_utils.h"

#include "types.h"
#include "OrganismBase.h"

namespace mabe {

  template <typename ENV_T> class Evolver;

  /// A class to track IDs (and thus vector choice and position) in Evolver.
  /// Note: Organisms may have IDs change during processing, but will remain unique.

  class EvolverID {
  private:
    static constexpr size_t invalid_id = (uint32_t) -1;
    uint32_t index;   ///<  Position of this organism in the population vector.
    uint32_t pop_id;  ///<  ID of the population we are in; 0 is always the active population.

  public:
    EvolverID() : index(invalid_id), pop_id(invalid_id) { ; }
    EvolverID(size_t _id, size_t _pop_id=0) : index((uint32_t) _id), pop_id((uint32_t) _pop_id) {
      emp_assert(_id <= invalid_id);
    }
    EvolverID(const EvolverID &) = default;

    uint32_t GetIndex() const { return index; }
    uint32_t GetPopID() const { return pop_id; }

    bool IsActive() const { return pop_id == 0; }
    bool IsValid() const { return index != invalid_id; }

    EvolverID & SetActive(bool _active=true) { pop_id = _active ? 0 : 1; return *this; }
    EvolverID & SetPopID(size_t _id) { emp_assert(_id <= invalid_id); pop_id = (uint32_t) _id; return *this; }
    EvolverID & SetIndex(size_t _id) { emp_assert(_id <= invalid_id); index = (uint32_t) _id; return *this; }
    EvolverID & MarkInvalid() { index = invalid_id; pop_id = invalid_id; return *this; }
  };


  /// A vector that can be indexed with an EvolverID
  class PopVector : public emp::array<emp::vector< emp::Ptr<mabe::OrganismBase> >, 2> {
  public:
    using ptr_t = emp::Ptr<mabe::OrganismBase>;
    using base_t = emp::array<emp::vector< ptr_t >, 2>;

    /// Test if an id is currently valid.
    bool IsValid(EvolverID evo_id) const {
      const size_t pop_id = evo_id.GetPopID();
      const size_t index = evo_id.GetIndex();
      emp_assert(pop_id < 2);
      return index < base_t::operator[](pop_id).size();
    }

    /// Make sure ID is in vector range; if not expand relevant vector.
    void MakeValid(EvolverID evo_id) {
      emp_assert(evo_id.IsValid());
      const size_t pop_id = evo_id.GetPopID();
      const size_t index = evo_id.GetIndex();
      emp_assert(pop_id < 2);
      if (index >= base_t::operator[](pop_id).size()) {
        base_t::operator[](pop_id).resize(index+1);
      }
    }

    ptr_t & operator()(EvolverID evo_id) {
      const size_t pop_id = evo_id.GetPopID();
      const size_t index = evo_id.GetIndex();
      return base_t::operator[](pop_id)[index];
    }
    const ptr_t & operator()(EvolverID evo_id) const {
      const size_t pop_id = evo_id.GetPopID();
      const size_t index = evo_id.GetIndex();
      return base_t::operator[](pop_id)[index];
    }
  };


  /// A base class for all MABE setup types, containing common functionality and all interfaces.
  class EvolverBase {
  public:
    using org_t = OrganismBase;           ///< Organisms are tracked by base classes.
    using value_type = OrganismBase;      ///< For compatability with vectors.
    using org_ptr_t = emp::Ptr<org_t>;    ///< To restore from base class, org pointers are used.
    using pop_t = emp::vector<org_ptr_t>; ///< Populations are tracked by vectors

  protected:
    /// Type of master Evolver config file.
    EMP_BUILD_CONFIG( base_config_t,
      GROUP(DEFAULT_GROUP, "Master Evolver Settings"),
      VALUE(RANDOM_SEED, int, 0, "Seed for main random number generator. Use 0 for based on time."),
      VALUE(INIT_SIZE, size_t, 1, "Initial population size for each organism type.")
    )

    base_config_t config;                                    ///< Master configuration object.

    emp::vector<emp::Ptr<OrganismTypeBase>> organism_types;  ///< Vector of organism-type modules. 
    emp::vector<emp::Ptr<SchemaBase>> schemas;               ///< Vector of schema modules. 

    // ----- Evolver STATE -----
    const std::string name;           ///< Unique name for this Evolver instance (for use in configuration.)
    size_t update;                    ///< How many times has Update() been called?
    emp::Random random;               ///< Random object to use.
    PopVector pops;                   ///< Set of active [0] and "next" [1] orgs in population.
    pop_t & active_pop;               ///< Shortcut to pops[0]
    pop_t & next_pop;                 ///< Shortcut to pops[1]
    size_t num_orgs;                  ///< How many organisms are actually in the population.
    emp::vector<double> fit_cache;    ///< vector size==0 when not caching; uncached values==0.0


    // ----- Evolver CONFIG ----
    bool cache_on;                    ///< Should we be caching fitness values?
    std::vector<size_t> pop_sizes;    ///< Sizes of population dimensions (eg, 2 vals for grid)
    emp::TraitSet<org_t> phenotypes;  ///< What phenotypes are we tracking?
    emp::vector<emp::Ptr<emp::DataFile>> files;    ///< Output files.

    OrgDataMap org_data_map;          ///< Details of how run data is stored in orgnisms.


    bool is_synchronous;              ///< Do we have synchronous generations?
    bool is_space_structured;         ///< Do we have a spatially structured population?
    bool is_pheno_structured;         ///< Do we have a phenotypically structured population?

    /// Function type for calculating fitness of organisms, typically set by the environment.
    using fun_calc_fitness_t = std::function<double(org_t&)>;
    fun_calc_fitness_t fun_calc_fitness;    
 
    /// Function type for a mutation operator on an organism.
    using fun_do_mutations_t = std::function<size_t(org_t&)>;
    fun_do_mutations_t fun_do_mutations;

    /// Function type for printing an organism's info to an output stream.
    using fun_print_org_t = std::function<void(org_t&, std::ostream &)>;
    fun_print_org_t fun_print_org;

    /// Function type for injecting organisms (returns inject EvolverID)
    using fun_find_inject_id_t = std::function<EvolverID(org_ptr_t)>;
    fun_find_inject_id_t fun_find_inject_id;

    /// Function type for adding a newly born organism (returns birth EvolverID)
    using fun_find_birth_id_t  = std::function<EvolverID(org_ptr_t, EvolverID)>;
    fun_find_birth_id_t fun_find_birth_id;

    /// Function type for determining picking and killing an organism (returns newly empty cell id)
    using fun_kill_org_t = std::function<EvolverID()>;
    fun_kill_org_t fun_kill_org;

    /// Function type for identifying a random neighbor "near" specified id.
    using fun_get_neighbor_t = std::function<EvolverID(EvolverID)>;
    fun_get_neighbor_t fun_get_neighbor;


    /// Attributes are a dynamic way to track extra characteristics about an Evolver instance.
    std::map<std::string, std::string> attributes;

    // @CAO: Still need to port over systematics!

    // == Signals ==
    emp::SignalControl control;  // Setup Evolver to control various signals.
    
    /// Trigger signal... before organism gives birth w/parent EvolverID.
    emp::Signal<void(size_t)> before_repro_sig;
    
    /// Trigger signal... when offspring organism is built.
    emp::Signal<void(org_t &,size_t)> offspring_ready_sig;
    
    /// Trigger signal... when outside organism is ready to inject.
    emp::Signal<void(org_t &)> inject_ready_sig;
    
    /// Trigger signal... before placing any organism into target cell.
    emp::Signal<void(org_t &,size_t)> before_placement_sig;
    
    /// Trigger signal... after any organism is placed into the population.
    emp::Signal<void(size_t)> on_placement_sig;
    
    /// Trigger signal... at the beginning of Update()
    emp::Signal<void(size_t)> on_update_sig;
    
    /// Trigger signal... immediately before any organism dies.
    emp::Signal<void(size_t)> on_death_sig;
    
    /// Trigger signal... if org EvolverIDs are swapped
    emp::Signal<void(EvolverID,EvolverID)> on_swap_sig;
    
    /// Trigger signal... in the Evolver destructor.
    emp::Signal<void()> on_destruct_sig;
  
  public:
    EvolverBase(const std::string & _name="Evolver")
      : config(), organism_types(), schemas()
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
      , on_destruct_sig(emp::to_string(name,"::wolrd-destruct"), control)
    {      
    }
    virtual ~EvolverBase() { ; }

    /// How many organisms can fit in the world?
    size_t GetSize() const { return pops[0].size(); }

    /// How many organisms are currently in the world?
    size_t GetNumOrgs() const { return num_orgs; }

    /// What update number are we currently on? (assumes Update() is being used)
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

    /// Add a new type of organism data to Evolver along with a function to calculate it.
    template <typename DATA_T, typename FUN_T>
    void AddOrgData(const std::string & name, DATA_T default_val, FUN_T fun,
                    const std::string & type_info="", const std::string & desc="") {
      org_data_map.Add<DATA_T>(name, default_val);
      (void) fun;
      (void) type_info;
      (void) desc;
    }

    /// Retrive organism data by name in a mutable format.
    template <typename T>
    T & GetOrgData(org_t & org, const std::string & name) {
      return org_data_map.Get<T>(org.GetData(), name);
    }

    /// Retrive organism data by name in a const format.
    template <typename T>
    const T & GetOrgData(org_t & org, const std::string & name) const {
      return org_data_map.Get<T>(org.GetData(), name);
    }

    /// Retrive organism data by id number in a mutable format.
    template <typename T>
    T & GetOrgData(org_t & org, size_t id) {
      return org_data_map.Get<T>(org.GetData().GetTuple(), id);
    }

    /// Retrive organism data by id number in a const format.
    template <typename T>
    const T & GetOrgData(org_t & org, size_t id) const {
      return org_data_map.Get<T>(org.GetData().GetTuple(), id);
    }

    size_t GetOrgDataID(const std::string & name) const {
      return org_data_map.GetID(name);
    }

    /// Retrive a full blob of organism data (for initializing new organisms).
    OrgDataBlob GetOrgDataBlob() const { return org_data_map.MakeBlob(); }


    /// Does the specified cell ID have an organism in it?
    bool IsOccupied(EvolverID evo_id) const { return pops.IsValid(evo_id) && pops(evo_id); }

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

    /// Denote that we are using synchronous generations.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    void MarkSynchronous(bool in=true) { is_synchronous = in; }

    /// Denote that we are using a spatial structure for the organisms.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    void MarkSpaceStructured(bool in=true) { is_space_structured = in; }

    /// Denote that we have organisms structured based on phenotype.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    void MarkPhenoStructured(bool in=true) { is_pheno_structured = in; }

    /// Index into Evolver population to obtain a const reference to an organism.
    /// Any manipulations to organisms should go through other functions to be tracked
    /// appropriately.  Will trip assert if cell is not occupied.
    const org_t & operator[](size_t id) const {
      emp_assert(active_pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[0][id]);
    }

    /// Retrieve a reference to the organsim with the specified id.
    /// Same as operator[]; will trip assert if cell is not occupied.
    org_t & GetOrg(size_t id) {
      emp_assert(id < active_pop.size());         // Pop must be large enough.
      emp_assert(active_pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[0][id]);
    }

    /// Retrieve a const reference to the organsim as the specified x,y coordinates.
    /// @CAO: Technically, we should set this up with any number of coordinates.
    org_t & GetOrg(size_t x, size_t y) { return GetOrg(x+y*GetWidth()); }

    /// Retrive a pointer to the contents of a speciefied cell; will be nullptr if the cell is
    /// not occupied.
    const org_ptr_t GetOrgPtr(size_t id) const { return pops[0][id]; }

    /// Retrieve a reference to the organsim at the specified cell id in the NEXT population.
    /// Will trip assert if cell is not occupied.
    org_t & GetNextOrg(size_t id) {
      emp_assert(id < pops[1].size());         // Next pop must be large enough.
      emp_assert(pops[1][id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[1][id]);
    }

    void Clear() {
      for (size_t pop_id = 0; pop_id < 2; pop_id++) {
        for (size_t i = 0; i < pops[pop_id].size(); i++) RemoveOrgAt(EvolverID(i,pop_id));
        pops[pop_id].resize(0);
      }
    }

    /// Clear all of the orgs and reset stats.
    void Reset() { Clear(); update = 0; }

    /// Swap the IDs of two organisms.
    void Swap(EvolverID id1, EvolverID id2) {
      std::swap(pops(id1), pops(id2));
      on_swap_sig.Trigger(id1, id2);
    }

    /// Change the size of the population.  If the new size is smaller than the old, remove any
    /// organisms outside the new range.  If larger, new cells are empty.
    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pops[0].size(); i++) RemoveOrgAt(i); // Remove orgs past new size.
      pops[0].resize(new_size, nullptr);                                 // Default new orgs to null.
    }

    /// Change the size of the population based on width and height.
    void Resize(size_t new_width, size_t new_height) {
      Resize(new_width * new_height);
      pop_sizes.resize(2);
      pop_sizes[0] = new_width; pop_sizes[1] = new_height;
    }

    /// Change the size of the population based on a vector of dimensions.
    void Resize(const emp::vector<size_t> & dims) {
      Resize(emp::Product(dims));
      pop_sizes = dims;
    }

    /// AddOrgAt is the core function to add organisms to the population (others must go through here)
    /// Note: This function ignores population structure, so requires you to manage your own structure.
    void AddOrgAt(org_ptr_t new_org, EvolverID id, EvolverID p_id=EvolverID());

    /// RemoveOrgAt is the core function to remove organisms from the population.
    /// Note: This function ignores population structure, so requires you to manage your own structure.
    void RemoveOrgAt(EvolverID id);

    /// Inject an organism using the default injection scheme.
    void Inject(org_ptr_t new_org, size_t copy_count=1);
    void Inject(org_t & org, size_t copy_count=1) { Inject(org.Clone(), copy_count); }

    /// Inject an organism at a specific cell id.
    void InjectAt(org_ptr_t new_org, const EvolverID id);
    void InjectAt(org_t & org, const EvolverID id) { InjectAt(org.Clone(), id); }

    /// Place one or more copies of an offspring into population; return id of last placed.
    EvolverID DoBirth(org_ptr_t parent, size_t parent_id, size_t copy_count=1);

    // Kill off organism at the specified ID (same as RemoveOrgAt, but callable externally)
    void DoDeath(const EvolverID id) { RemoveOrgAt(id); }


    /// Get the id of a random *occupied* cell.
    size_t GetRandomOrgID() {
      emp_assert(num_orgs > 0); // Make sure it's possible to find an organism!
      size_t id = random.GetUInt(0, active_pop.size());
      while (active_pop[id] == nullptr) id = random.GetUInt(0, active_pop.size());
      return id;
    }

    /// Get an organism from a random occupied cell.
    org_t & GetRandomOrg() { return *active_pop[GetRandomOrgID()]; }



    /// Run should be called when Evolver is configured and ready to go.  It will initialize the
    /// population (if needed) and run updates until finished producing a return code for main().
    int Run();
  };


  // ==============================================
  // ===                                        ===
  // ===       class Evolver<environment>       ===
  // ===                                        ===
  // ==============================================


  template <typename ENV_T>
  class Evolver : public EvolverBase {
  public:
    using env_t = ENV_T;              ///< Specify the environment type.

  private:
    // ----- Evolver MODULES -----
    env_t environment;    ///< Current environment. 

  public:
    Evolver(const std::string & _name="Evolver")
      : EvolverBase(_name), environment(_name)
    {
      config.AddNameSpace(environment.GetConfig(), name);   // Setup environment config in a namespace.
    }

    ~Evolver() {
      // Remove all organisms.
      Clear();

      // Triger the signal to indicate that Evolver is being destroyed.
      on_destruct_sig.Trigger();

      // Clean up all allocated pointers.
      for (emp::Ptr<ModuleBase> x : organism_types)  { x.Delete(); }
      for (emp::Ptr<ModuleBase> x : schemas)         { x.Delete(); }
      for (auto file : files) file.Delete();
    }

    env_t & GetEnvironment() { return environment; }

    /// Build a new organism type module.    
    template <typename T>
    T & AddOrgType(const std::string name) {
      emp::Ptr<T> org_mod = emp::NewPtr<T>(name);       // Build the new module.
      organism_types.push_back(org_mod);                // Store new module in org type vector.
      config.AddNameSpace(org_mod->GetConfig(), name);  // Setup module's config in a namespace.
      return *org_mod;                                  // Return the final module.
    }

    /// Build a new schema module.    
    template <typename T>
    T & AddSchema(const std::string name) {
      emp::Ptr<T> new_mod = emp::NewPtr<T>(name);       // Build the new module.
      schemas.push_back(new_mod);                       // Store new module in schema vector.
      config.AddNameSpace(new_mod->GetConfig(), name);  // Setup module's config in a namespace.
      return *new_mod;                                  // Return the final module.
    }

    bool Config(int argc, char * argv[], const std::string & filename,
                const std::string & macro_filename="") {
      config.Read(filename, false);
      auto args = emp::cl::ArgManager(argc, argv);
      bool config_continue = args.ProcessConfigOptions(config, std::cout, filename, macro_filename);

      if (!config_continue) exit(0);  // Exit if config is supposed to stop (e.g., --gen)
      if (args.HasUnknown()) exit(1); // Exit if there were unknown command line args provided.

      // Setup Evolver with Config options.
      random.ResetSeed(config.RANDOM_SEED());

      // Now that all of the modules have been configured, allow them to setup Evolver.
      environment.Setup(*this);
      for (emp::Ptr<OrganismTypeBase> x : organism_types) { x->Setup(*this); }
      for (emp::Ptr<SchemaBase> x : schemas) { x->Setup(*this); }

      return true;
    }
    

    void PrintStatus() {
      std::cout << "Environment: " << environment.GetName()
                << " (class name: " << environment.GetClassName() << ")" << std::endl;
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

  void EvolverBase::AddOrgAt(org_ptr_t new_org, EvolverID id, EvolverID p_id) {
    emp_assert(new_org);         // The new organism must exist.
    emp_assert(id.IsValid());   // Position must be legal.

    // If new organism is going into the active population, trigger signal before doing so.
    if (id.IsActive()) { before_placement_sig.Trigger(*new_org, id.GetIndex()); }

    // for (Ptr<SystematicsBase<ORG> > s : systematics) {
    //   s->SetNextParent((int) p_id.GetIndex());
    // }

    // Clear out any old organism at this cell id.
    RemoveOrgAt(id);

    pops.MakeValid(id);                 // Make sure we have room for new organism
    pops(id) = new_org;                 // Put org into place.

    // Track org count
    if (id.IsActive()) ++num_orgs;

    // Track the new systematics info
    // for (Ptr<SystematicsBase<ORG> > s : systematics) {
    //   s->AddOrg(*new_org, (int) id.GetIndex(), (int) update, !id.IsActive());
    // }

    // new_org->Setup(id, random);

    // If new organism is in the active population, trigger associated signal.
    if (id.IsActive()) { on_placement_sig.Trigger(id.GetIndex()); }
  }

  void EvolverBase::RemoveOrgAt(EvolverID evo_id) {
    size_t id = evo_id.GetIndex();                     // Identify specific index.
    pop_t & cur_pop = pops[evo_id.GetPopID()];
    if (id >= cur_pop.size() || !cur_pop[id]) return;  // Nothing to remove!
    if (evo_id.IsActive()) on_death_sig.Trigger(id);   // If active, signal that org is about to die.
    cur_pop[id].Delete();                              // Delete the organism...
    cur_pop[id] = nullptr;                             // ...and reset the pointer to null

    if (evo_id.IsActive()) {
      --num_orgs;                                    // Track one fewer organisms in the population
      // if (cache_on) ClearCache(id);                  // Delete any cached info about this organism
      // for (Ptr<SystematicsBase<ORG> > s : systematics) {
      //   s->RemoveOrg((int) evo_id.GetIndex());          // Notify systematics about organism removal
      // }
    } else {
      // for (Ptr<SystematicsBase<ORG> > s : systematics) {
      //   s->RemoveNextOrg((int) evo_id.GetIndex());      // Notify systematics about organism removal
      // }
    }
  }

  void EvolverBase::Inject(org_ptr_t new_org, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      inject_ready_sig.Trigger(*new_org);
      const EvolverID id = fun_find_inject_id(new_org);

      if (id.IsValid()) AddOrgAt(new_org, id);  // If placement cell id is valid, do so!
      else new_org.Delete();                    // Otherwise delete the organism.
    }
  }

  void EvolverBase::InjectAt(org_ptr_t new_org, const EvolverID id) {
    emp_assert(id.IsValid());
    inject_ready_sig.Trigger(*new_org);
    AddOrgAt(new_org, id);
  }

  // Give birth to (potentially) multiple offspring; return cell id of last placed.
  // Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
  // Additional signal triggers occur in AddOrgAt.
  // @CAO: NOTE Parent may die during multi-birth; should delay destruction until after DoBirth.
  // @CAO: NOTE That this DoBirth assume asexual reproduction; need another version!
  EvolverID EvolverBase::DoBirth(org_ptr_t parent_ptr, size_t parent_id, size_t copy_count) {
    before_repro_sig.Trigger(parent_id);
    EvolverID id;                                            // Position of each offspring placed.
    for (size_t i = 0; i < copy_count; i++) {                // Loop through offspring, adding each
      org_ptr_t new_org = parent_ptr->Clone();               // Offspring is initially clone of parent.
      offspring_ready_sig.Trigger(*new_org, parent_id);
      id = fun_find_birth_id(new_org, parent_id);

      if (id.IsValid()) AddOrgAt(new_org, id, parent_id);    // If placement cell id is valid, do so!
      else new_org.Delete();                                 // Otherwise delete the organism.
    }
    return id;
  }


  int EvolverBase::Run() {
    /// Make sure all OrganismTypes have been initialized with organisms
    for (auto & org_type_ptr : organism_types) {
      while (org_type_ptr->GetCount() < config.INIT_SIZE()) {
        org_ptr_t new_org = org_type_ptr->BuildOrg(random);
        std::cout << "Org: ";
        org_type_ptr->Print(std::cout, *new_org);
        std::cout << std::endl;
      }
    }

    return 0;
  }
}

#endif

