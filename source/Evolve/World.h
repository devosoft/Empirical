/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  World.h
 *  @brief Definition of a base class for a World template for use in evolutionary algorithms.
 *
 *  A definition of the emp::World template, linking in specialized file handling, iterators,
 *  and selection techniques for evolutionary computation applications.
 *
 *
 *  @todo Make sure when mutations occure before placement into the population we can control
 *        whether or not they also affect injected organisms.  (Right now they alwyas do!!)
 *  @todo We should Specialize World so that ANOTHER world can be used as an ORG, with proper
 *        delegation to facilitate demes, pools, islands, etc.
 *  @todo We should be able to have any number of systematics managers, based on various type_trait
 *        information a that we want to track.
 *  @todo Add a signal for DoBirth() for when a birth fails.
 *  @todo Add a signal for population Reset() (and possibly Clear?)
 *  @todo Add a feature to maintain population sorted by each phenotypic trait.  This will allow
 *        us to more rapidly find phenotypic neighbors and know the current extremes for each
 *        phenotype.
 */

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <functional>
#include <map>
#include <unordered_map>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../control/Signal.h"
#include "../control/SignalControl.h"
#include "../data/DataFile.h"
#include "../data/DataManager.h"
#include "../data/Trait.h"
#include "../meta/reflection.h"
#include "../tools/map_utils.h"
#include "../tools/Random.h"
#include "../tools/Range.h"
#include "../tools/random_utils.h"
#include "../tools/string_utils.h"

// World-specific includes.
#include "Systematics.h"     // Track relationships among organisms.
#include "World_iterator.h"  // Allow iteration through organisms in a world.
#include "World_reflect.h"   // Handle needed reflection on incoming organism classes.
#include "World_select.h"    // Include all built-in selection functions for World.
#include "World_structure.h" // Include additional function to setup world structure.

namespace emp {

  ///  @brief Setup a World with a population of organisms that can evolve or deal with ecological effects.
  ///
  ///  There are three ways that organisms can enter the population:
  ///   * InjectAt(org, pos) - place the organism at the specified position in the population.
  ///   * Inject(org) - place the organism using a default postion (given other settings).
  ///   * DoBirth(org, parent_pos) - place the organism using current birth settings.
  ///
  ///  If the population is in EA mode (with synchronous generations), DoBirth will place offspring in
  ///  a "next generation" placeholder population.  Update() will move orgs into primary population.
  ///
  ///  Organisms have a series of functions that are called on them that are chosen:
  ///
  ///  FITNESS: Most selection methods require a fitness function to help determine who should be
  ///           replicated.  Other systems merely use fitness as a measured output.
  ///   0. If you explicitly set the fitness function using SetFitFun(), it will have priority.
  ///   1. If the organism type has a "GetFitness()" member function, use it!
  ///   2. If the organism type can be cast to double, use it!
  ///   3. Start with a fitness function that throws an assert indicating function must be set.
  ///
  ///  MUTATIONS: The mutation function deteramines a main source of variation for most evolving
  ///             systems.
  ///   0. If you set the mutation function using SetMutFun(), it will have priority.
  ///   1. Or DoMutations(random) member function.
  ///   2. Empty, with assert.
  ///
  ///  PRINTING: How should organisms be printed to the command line?
  ///   0. Setting the print function with SetPrintFun() member function.
  ///   1. Org Print() member function that takes an ostream & argument
  ///   2. Proper operator<<
  ///   3. Do not print, just Assert
  ///
  ///  GENOMES: Do organisms have a genome separate from their instantiation?  By default, the full
  ///           organism is returned when a genome is requested, but a GetGenome() member function
  ///           in the organism type will override this behavior.
  ///   1. GetGenome member function
  ///   2. Return org AS genome.

  template <typename ORG>
  class World {
    friend class World_iterator< World<ORG> >;
  public:
    // --- Publicly available types ---
    using this_t = World<ORG>;                 ///< Resolved type of this templated class.
    using org_t = ORG;                         ///< Type of organisms in this world.
    using value_type = org_t;                  ///< Identical to org_t; vector compatibility.
    using pop_t = emp::vector<Ptr<ORG>>;       ///< Type for whole populations.
    using iterator_t = World_iterator<this_t>; ///< Type for this world's iterators.
    using fit_cache_t = emp::vector<double>;   ///< Type for fitness caches for pops.

    using genome_t = typename emp::find_genome_t<ORG>;   ///< Type of underlying genomes.

    /// Function type for calculating fitness.
    using fun_calc_fitness_t    = std::function<double(ORG&)>;

    /// Function type for calculating the distance between two organisms.
    using fun_calc_dist_t       = std::function<double(ORG&,ORG&)>;

    /// Function type for a mutation operator on an organism.
    using fun_do_mutations_t    = std::function<size_t(ORG&,Random&)>;

    /// Function type for printing an organism's info to an output stream.
    using fun_print_org_t       = std::function<void(ORG&,std::ostream &)>;

    /// Function type for retrieving a genome from an organism.
    using fun_get_genome_t      = std::function<const genome_t & (ORG &)>;

    /// Function type for injecting organisms into a world (returns inject position)
    using fun_find_inject_pos_t = std::function<WorldPosition(Ptr<ORG>)>;

    /// Function type for adding a newly born organism into a world (returns birth position)
    using fun_find_birth_pos_t  = std::function<WorldPosition(Ptr<ORG>, WorldPosition)>;

    /// Function type for determining picking and killing an organism (returns newly empty position)
    using fun_kill_org_t        = std::function<WorldPosition()>;

    /// Function type for identifying an organism's random neighbor.
    using fun_get_neighbor_t    = std::function<WorldPosition(WorldPosition)>;

  protected:
    // Internal state member variables
    size_t update;                  ///< How many times has Update() been called?
    Ptr<Random> random_ptr;         ///< Random object to use.
    bool random_owner;              ///< Did we create our own random number generator?
    WorldVector<Ptr<ORG>> pops;     ///< The set of active [0] and "next" [1] organisms in population.
    pop_t & pop;                    ///< A shortcut to pops[0].
    size_t num_orgs;                ///< How many organisms are actually in the population.
    fit_cache_t fit_cache;          ///< vector size == 0 when not caching; uncached values == 0.

    // Configuration settings
    std::string name;               ///< Name of this world (for use in configuration.)
    bool cache_on;                  ///< Should we be caching fitness values?
    std::vector<size_t> pop_sizes;  ///< Sizes of population dimensions (eg, 2 vals for grid)
    emp::TraitSet<ORG> phenotypes;  ///< What phenotypes are we tracking?
    emp::vector<emp::Ptr<DataFile>> files;    ///< Output files.

    bool is_synchronous;            ///< Does this world have synchronous generations?
    bool is_space_structured;       ///< Do we have a spatially structured population?
    bool is_pheno_structured;       ///< Do we have a phenotypically structured population?

    /// Potential data nodes -- these should be activated only if in use.
    DataManager<double, data::Current, data::Info, data::Range, data::Stats> data_nodes;

    // Configurable functions.                       Function to...
    fun_calc_fitness_t     fun_calc_fitness;    ///< ...evaluate fitness for provided organism.
    fun_do_mutations_t     fun_do_mutations;    ///< ...mutate an organism.
    fun_print_org_t        fun_print_org;       ///< ...print an organism.
    fun_get_genome_t       fun_get_genome;      ///< ...determine the genome object of an organism.
    fun_find_inject_pos_t  fun_find_inject_pos; ///< ...find where to inject a new, external organism.
    fun_find_birth_pos_t   fun_find_birth_pos;  ///< ...find where to add a new offspring organism.
    fun_kill_org_t         fun_kill_org;        ///< ...kill an organism.
    fun_get_neighbor_t     fun_get_neighbor;    ///< ...choose a random neighbor "near" specified id.

    /// Attributes are a dynamic way to track extra characteristics about a world.
    std::map<std::string, std::string> attributes;

    /// Phylogeny and line-of-descent data collection.
    emp::vector<Ptr<SystematicsBase<ORG> >> systematics;
    std::unordered_map<std::string, int> systematics_labels;

    // == Signals ==
    SignalControl control;  // Setup the world to control various signals.
                                                     //   Trigger signal...
    Signal<void(size_t)>       before_repro_sig;     ///< ...before organism gives birth w/parent position.
    Signal<void(ORG &,size_t)> offspring_ready_sig;  ///< ...when offspring organism is built.
    Signal<void(ORG &)>        inject_ready_sig;     ///< ...when outside organism is ready to inject.
    Signal<void(ORG &,size_t)> before_placement_sig; ///< ...before placing any organism into target cell.
    Signal<void(size_t)>       on_placement_sig;     ///< ...after any organism is placed into world.
    Signal<void(size_t)>       on_update_sig;        ///< ...at the beginning of Update()
    Signal<void(size_t)>       on_death_sig;         ///< ...immediately before any organism dies.
    Signal<void(WorldPosition,WorldPosition)> on_swap_sig; ///< ...after org positions are swapped
    Signal<void()>             world_destruct_sig;   ///< ...in the World destructor.

    /// Build a Setup function in world that calls ::Setup() on whatever is passed in IF it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);

    /// Get the current cached value for the organism at the specified position.
    double GetCache(size_t id) const { return (id < fit_cache.size()) ? fit_cache[id] : 0.0; }

    /// Clear the cache value at the specified position.
    void ClearCache(size_t id) { if (id < fit_cache.size()) fit_cache[id] = 0.0; }

  public:
    /// The World constructor can take two arguments, both optional:
    /// * a random number generator (either a pointer or reference)
    /// * a unique name for the world
    /// If no name is provided, the world remains nameless.
    /// If no random number generator is provided, gen_random determines if one shold be created.
    World(std::string _name="", bool gen_random=true)
      : update(0), random_ptr(nullptr), random_owner(false), pops(), pop(pops[0]), num_orgs(0)
      , fit_cache()
      , name(_name), cache_on(false), pop_sizes(1,0), phenotypes(), files()
      , is_synchronous(false), is_space_structured(false), is_pheno_structured(false)
      , fun_calc_fitness(), fun_do_mutations(), fun_print_org(), fun_get_genome()
      , fun_find_inject_pos(), fun_find_birth_pos(), fun_kill_org(), fun_get_neighbor()
      , attributes(), control()
      , before_repro_sig(to_string(name,"::before-repro"), control)
      , offspring_ready_sig(to_string(name,"::offspring-ready"), control)
      , inject_ready_sig(to_string(name,"::inject-ready"), control)
      , before_placement_sig(to_string(name,"::before-placement"), control)
      , on_placement_sig(to_string(name,"::on-placement"), control)
      , on_update_sig(to_string(name,"::on-update"), control)
      , on_death_sig(to_string(name,"::on-death"), control)
      , on_swap_sig(to_string(name,"::on-swap"), control)
      , world_destruct_sig(to_string(name,"::wolrd-destruct"), control)
    {
      if (gen_random) NewRandom();
      SetDefaultFitFun<this_t, ORG>(*this);
      SetDefaultMutFun<this_t, ORG>(*this);
      SetDefaultPrintFun<this_t, ORG>(*this);
      SetDefaultGetGenomeFun<this_t, ORG>(*this);
      SetPopStruct_Mixed();  // World default structure is well-mixed.
    }
    World(Random & rnd, std::string _name="") : World(_name,false) { random_ptr = &rnd; }

    ~World() {
      world_destruct_sig.Trigger();
      Clear();
      if (random_owner) random_ptr.Delete();
      for (Ptr<SystematicsBase<ORG> > s : systematics) {
        s.Delete();
      }
      for (auto file : files) {
        file.Delete();
      }
    }

    // --- Accessing Organisms or info ---

    /// How many organisms can fit in the world?
    size_t GetSize() const { return pop.size(); }

    /// How many organisms are currently in the world?
    size_t GetNumOrgs() const { return num_orgs; }

    /// What update number is the world currently on? (assumes Update() is being used)
    size_t GetUpdate() const { return update; }

    /// How many cells wide is the world? (assumes grids are active.)
    size_t GetWidth() const { return pop_sizes[0]; }

    /// How many cells tall is the world? (assumes grids are active.)
    size_t GetHeight() const { return pop_sizes[1]; }

    /// Get the full population to analyze externally.
    const pop_t & GetFullPop() const { return pop; }

    /// What phenotypic traits is the population tracking?
    const emp::TraitSet<ORG> & GetPhenotypes() const { return phenotypes; }

    /// Add an already-constructed datafile.
    DataFile & AddDataFile(emp::Ptr<DataFile> file);

    /// Lookup a file by name.
    DataFile & GetFile(const std::string & filename) {
      for (emp::Ptr<DataFile> file : files) {
        if (file->GetFilename() == filename) return *file;
      }
      emp_assert(false, "Trying to lookup a file that does not exist.", filename);
    }

    /// Does the specified cell ID have an organism in it?
    bool IsOccupied(WorldPosition pos) const { return pops.IsValid(pos) && pops(pos); }

    /// Are we currently caching fitness values?
    bool IsCacheOn() const { return cache_on; }

    /// Are generations being evaluated synchronously?
    /// (i.e., Update() places all births into the population after removing all current organisms.)
    bool IsSynchronous() const { return is_synchronous; }

    /// Is there some sort of spatial structure to the population?
    /// (i.e., are some organisms closer together than others.)
    bool IsSpaceStructured() const { return is_space_structured; }

    /// Is there some sort of structure to the population based on phenotype?
    /// (i.e., are phenotypically-similar organisms forced to be closer together?)
    bool IsPhenoStructured() const { return is_pheno_structured; }

    /// Denote that this World will be treated as having synchronous generations.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    this_t & MarkSynchronous(bool in=true) { is_synchronous = in; return *this; }

    /// Denote that the World will have a spatial structure to the organisms.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    this_t & MarkSpaceStructured(bool in=true) { is_space_structured = in; return *this; }

    /// Denote that the World will have organisms structured based on phenotype.
    /// (Note: this function does not change functionality, just indicates what's happening!)
    this_t & MarkPhenoStructured(bool in=true) { is_pheno_structured = in; return *this; }

    /// Index into a world to obtain a const reference to an organism.  Any manipulations to
    /// organisms should go through other functions to be tracked appropriately.
    /// Will trip assert if cell is not occupied.
    const ORG & operator[](size_t id) const {
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pop[id]);
    }

    /// Retrieve a reference to the organsim as the specified position.
    /// Same as operator[]; will trip assert if cell is not occupied.
    ORG & GetOrg(size_t id) {
      emp_assert(id < pop.size());         // Pop must be large enough.
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pop[id]);
    }

    /// Retrieve a const reference to the organsim as the specified x,y coordinates.
    /// @CAO: Technically, we should set this up with any number of coordinates.
    ORG & GetOrg(size_t x, size_t y) { return GetOrg(x+y*GetWidth()); }

    /// Retrive a pointer to the contents of a speciefied cell; will be nullptr if the cell is
    /// not occupied.
    const Ptr<ORG> GetOrgPtr(size_t id) const { return pop[id]; }

    /// Retrieve a reference to the organsim as the specified position in the NEXT population.
    /// Will trip assert if cell is not occupied.
    ORG & GetNextOrg(size_t id) {
      emp_assert(id < pops[1].size());         // Next pop must be large enough.
      emp_assert(pops[1][id] != nullptr, id);  // Should not index to a null organism!
      return *(pops[1][id]);
    }

    /// Retrieve the genome corresponding to a specified organism.
    const genome_t & GetGenome(ORG & org) { return fun_get_genome(org); }

    /// Retrive the genome corresponding to the organism at the specified position.
    const genome_t & GetGenomeAt(size_t id) { return fun_get_genome(GetOrg(id)); }

    /// Get a systematics manager (which is tracking lineages in the population.)
    /// @param id - which systematics manager to return? Systematics managers are
    /// stored in the order they are added to the world.
    Ptr<SystematicsBase<ORG> > GetSystematics(int id=0) {
      emp_assert(systematics.size() > 0, "Cannot get systematics file. No systematics file to track.");
      emp_assert(id < (int)systematics.size(), "Invalid systematics file requested.", id, systematics.size());
      return systematics[id];
    }

    /// Get a systematics manager (which is tracking lineages in the population.)
    /// @param id - which systematics manager to return? Systematics managers are
    /// stored in the order they are added to the world.
    Ptr<SystematicsBase<ORG> > GetSystematics(std::string label) {
      emp_assert(Has(systematics_labels, label), "Invalid systematics manager label");

      return systematics[systematics_labels[label]];
    }


    void RemoveSystematics(int id) {
      emp_assert(systematics.size() > 0, "Cannot remove systematics file. No systematics file to track.");
      emp_assert(id < systematics.size(), "Invalid systematics file requested to be removed.", id, systematics.size());

      systematics[id].Delete();
      systematics[id] = nullptr;

      for (auto el : systematics_labels) {
        if (el.second == id) {
          systematics_labels.erase(el.first);
        }
      }
    }

    void RemoveSystematics(std::string label) {
      emp_assert(Has(systematics_labels, label), "Invalid systematics manager label");

      systematics[systematics_labels[label]].Delete();
      systematics[systematics_labels[label]] = nullptr;
      systematics_labels.erase(label) ;
    }

    template <typename ORG_INFO, typename DATA_STRUCT>
    void AddSystematics(Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT> > s, std::string label="systematics") {
      if (Has(systematics_labels, label)) {
        label += to_string(systematics.size());
      }
      systematics_labels[label] = systematics.size();

      if (is_synchronous) {
        s->SetTrackSynchronous(true);
      }

      systematics.push_back(s);
    }

    /// Get the fitness function currently in use.
    fun_calc_fitness_t GetFitFun() { return fun_calc_fitness; }

    // --- CONFIGURE ---

    /// Set the population to always append new organisms on the end.
    /// Argument determines if the generations should be synchronous (true) or not (false, default)
    void SetPopStruct_Grow(bool synchronous_gen=false);

    /// Set the population to be well-mixed (with all organisms counting as neighbors.)
    /// Argument determines if the generations should be synchronous (true) or not (false, default)
    void SetPopStruct_Mixed(bool synchronous_gen=false);

    /// Set the population to be a grid of cells using the specified dimensions.  The third
    /// argument determines if the generations should be synchronous (true) or not (false, default)
    void SetPopStruct_Grid(size_t width, size_t height, bool synchronous_gen=false);

    /// Setup the population to automatically test for and trigger mutations.  By default, this
    /// occurs before deciding where an offspring should be placed. Note that this pre-placement
    /// timing may be needed if fitness or other phenotypic traits are required to determine placement.
    void SetAutoMutate() {
      OnOffspringReady( [this](ORG & org, size_t){ DoMutationsOrg(org); } );
    }

    /// Setup the population to automatically test for and trigger mutations based on a provided
    /// test function that takes the position where the offspring will be placed and indicates
    /// (true/false) whether mutations should be processed.  This timing allows position to
    /// influence mutations.
    void SetAutoMutate(std::function<bool(size_t pos)> test_fun) {
      OnBeforePlacement( [this,test_fun](ORG & org, size_t pos){ if (test_fun(pos)) DoMutationsOrg(org); } );
    }

    /// Tell systematics managers that this world has synchronous generations.
    //  @ELD: Can probably be removed with new WorldPosition
    void SetSynchronousSystematics(bool synchronous) {
      for (Ptr<SystematicsBase<ORG> > s : systematics) {
        s->SetTrackSynchronous(synchronous);
      }
    }

    /// Add a new phenotype measuring function.
    // void AddPhenotype(const std::string & name, std::function<double(ORG &)> fun) {
    //   phenotypes.AddTrait(name, fun);
    // }
    template <typename... Ts>
    void AddPhenotype(Ts &&... args) {
      phenotypes.AddTrait(std::forward<Ts>(args)...);
    }

    /// Access a data node that tracks fitness information in the population.  The fitness will not
    /// be collected until the first Update() after this function is initially called, signaling
    /// the need for this information.
    Ptr<DataMonitor<double>> GetFitnessDataNode() {
      if (!data_nodes.HasNode("fitness")) {
        DataMonitor<double> & node = data_nodes.New("fitness");

        // Collect fitnesses each update...
        OnUpdate([this, &node](size_t){
            node.Reset();
            for (size_t i = 0; i < pop.size(); i++) {
              if (IsOccupied(i)) node.AddDatum( CalcFitnessID(i) );
            }
          }
        );
      }
      return &(data_nodes.Get("fitness"));
    }

    // Returns a reference so that capturing it in a lambda to call on update
    // is less confusing. It's possible we should change it to be consistent
    // with GetFitnessDataNode, though.
    Ptr<DataMonitor<double>> AddDataNode(const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      return &(data_nodes.New(name));
    }

    Ptr<DataMonitor<double>> GetDataNode(const std::string & name) {
      return &(data_nodes.Get(name));
    }

    /// Setup an arbitrary file; no default filename available.
    DataFile & SetupFile(const std::string & filename);

    /// Setup a file to be printed that collects fitness information over time.
    DataFile & SetupFitnessFile(const std::string & filename="fitness.csv", const bool & print_header=true);

    /// Setup a file (by label) to be printed that collects systematics information over time.
    DataFile & SetupSystematicsFile(std::string label, const std::string & filename="systematics.csv", const bool & print_header=true);

    /// Setup a file (by id) to be printed that collects systematics information over time.
    DataFile & SetupSystematicsFile(size_t id=0, const std::string & filename="systematics.csv", const bool & print_header=true);

    /// Setup a file to be printed that collects population information over time.
    DataFile & SetupPopulationFile(const std::string & filename="population.csv", const bool & print_header=true);

    /// Setup the function to be used when fitness needs to be calculated.  The provided function
    /// should take a reference to an organism and return a fitness value of type double.
    void SetFitFun(const fun_calc_fitness_t & fit_fun) { fun_calc_fitness = fit_fun; }

    /// Setup the function to be used to mutate an organism.  It should take a reference to an
    /// organism and return the number of mutations that occurred.
    void SetMutFun(const fun_do_mutations_t & mut_fun) { fun_do_mutations = mut_fun; }

    /// Setup the function to be used to print an organism.  It should take a reference to an
    /// organism and an std::ostream, with a void return.  The organism should get printed to
    /// the provided ostream.
    void SetPrintFun(const fun_print_org_t & print_fun) { fun_print_org = print_fun; }

    /// Setup the function to extract or convert an organism to a genome.  It should take an
    /// organism reference and return a const genome reference.
    void SetGetGenomeFun(const fun_get_genome_t & _fun) { fun_get_genome = _fun; }

    /// Setup the function to inject an organism into the population.  It should take a pointer
    /// to the organism to be injected and return a WorldPosition indicating where it was placed.
    void SetAddInjectFun(const fun_find_inject_pos_t & _fun) { fun_find_inject_pos = _fun; }

    /// Setup the function to place a newly born organism into the population.  It should take a
    /// pointer to the new organism and the position of the parent, returning a WorldPosition
    /// indicating where it was placed.
    void SetAddBirthFun(const fun_find_birth_pos_t & _fun) { fun_find_birth_pos = _fun; }

    /// Setup the function to kill an organism.  It should return a WorldPosition indicating
    /// the newly empty cell, which is not necessarily where the kill occurred.
    void SetKillOrgFun(const fun_kill_org_t & _fun) { fun_kill_org = _fun; }

    /// Setup the function to take an organism position id and return a random neighbor id from
    /// the population.
    void SetGetNeighborFun(const fun_get_neighbor_t & _fun) { fun_get_neighbor = _fun; }

    /// Same as setting a fitness function, but uses Goldberg and Richardson's fitness sharing
    /// function (1987) to make similar organisms detract from each other's fitness and prevent
    /// the population from clustering around a single peak.  In addition to the base fitness
    /// function, a shared fitness function also requires:
    ///  * a distance function that takes references to two organisms and returns a double
    ///    indicating the distance between those organisms,
    ///  * a sharing threshold (sigma share) that defines the maximum distance at which members
    ///    should be consdered in the same niche,
    ///  * and a value of alpha, which controls the shape of the fitness sharing curve.
    void SetSharedFitFun(const fun_calc_fitness_t & fit_fun, const fun_calc_dist_t & dist_fun,
                         double sharing_threshold, double alpha);

    // Deal with Signals

    /// Access signal controller used for this world directly.
    SignalControl & GetSignalControl() { return control; }


    /// Provide a function for World to call each time an organism is about to give birth.
    /// Trigger:  Immediately prior to parent producing offspring
    /// Argument: World ID for the parent-to-be
    /// Return:   Key value needed to make future modifications.
    SignalKey OnBeforeRepro(const std::function<void(size_t)> & fun) {
      return before_repro_sig.AddAction(fun);
    }

    /// Provide a function for World to call after an offspring organism has been created, but
    /// before it is inserted into the World.
    /// Trigger:  Offspring about to enter population
    /// Args:     Reference to organism about to be placed in population and position of parent.
    ///           (note: for multi-offspring orgs, parent may have been replaced already!)
    /// Return:   Key value needed to make future modifications.
    SignalKey OnOffspringReady(const std::function<void(ORG &,size_t)> & fun) {
      return offspring_ready_sig.AddAction(fun);
    }

    /// Provide a function for World to call before an external organim is injected into the World.
    /// Trigger:  New organism about to be added to population from outside
    /// Argument: Reference to organism about to be placed in population.
    /// Return:   Key value needed to make future modifications.
    SignalKey OnInjectReady(const std::function<void(ORG &)> & fun) {
      return inject_ready_sig.AddAction(fun);
    }

    /// Provide a function for World to call before an organism is added to the active population,
    /// but after position is found.  With synchonous generations, this occurs during Update().
    /// Trigger:  Organism is about to be added to population; position is known (either born or injected)
    /// Args:     (1) Reference to organism about to be placed; (2) Position organism will be placed.
    /// Return:   Key value needed to make future modifications.
    SignalKey OnBeforePlacement(const std::function<void(ORG &,size_t)> & fun) {
      return before_placement_sig.AddAction(fun);
    }

    /// Provide a function for World to call immediately after any organism has been added to the
    /// active population.  With synchonous generations, this occurs during Update().
    /// Trigger:  Organism has been added to population (either born or injected)
    /// Argument: Position of organism placed in the population.
    /// Return:   Key value needed to make future modifications.
    SignalKey OnPlacement(const std::function<void(size_t)> & fun) {
      return on_placement_sig.AddAction(fun);
    }

    /// Provide a function for World to call each time Update() is run.
    /// Trigger:  New update is starting
    /// Argument: Update number (sequentially increasing)
    /// Return:   Key value needed to make future modifications.
    SignalKey OnUpdate(const std::function<void(size_t)> & fun) {
      return on_update_sig.AddAction(fun);
    }

    /// Provide a function for World to call each time an organism is about to die.
    /// Trigger:  Organism is about to be killed
    /// Argument: Position of organism about to die
    /// Return:   Key value needed to make future modifications.
    SignalKey OnOrgDeath(const std::function<void(size_t)> & fun) {
      return on_death_sig.AddAction(fun);
    }

    /// Provide a function for World to call each time two organisms swap positions in world.
    /// Trigger:   Organisms have just swapped positions.
    /// Arguments: Positions of the two organisms.
    /// Return:    Key value needed to make future modifications.
    SignalKey OnSwapOrgs(const std::function<void(WorldPosition,WorldPosition)> & fun) {
      return on_swap_sig.AddAction(fun);
    }

    /// Provide a function for World to call at the start of its destructor (for additional cleanup).
    /// Trigger:  Destructor has begun to execture
    /// Argument: None
    /// Return:   Key value needed to make future modifications.
    SignalKey OnWorldDestruct(const std::function<void()> & fun) {
      return world_destruct_sig.AddAction(fun);
    }

    // --- MANAGE ATTRIBUTES ---

    /// Worlds can have arbitrary attributes that can be set and changed dynamically.
    /// This function determines if an attribute exists, regardless of its value.
    bool HasAttribute(const std::string & name) const { return Has(attributes, name); }

    /// Get the value for an attribute that you know exists.
    std::string GetAttribute(const std::string) const {
      emp_assert( Has(attributes, name) );
      return Find(attributes, name, "UNKNOWN");
    }

    /// Set the value of a new attribute on this world.  If the attribute already exists, it will
    /// be updated.  If it doesn't exist, it will be added.
    template <typename T>
    void SetAttribute(const std::string & name, T && val) { attributes[name] = to_string(val); }


    // --- UPDATE THE WORLD! ---

    /// Update the world:
    /// 1. Send out an update signal for any external functions to trigger.
    /// 2. If synchronous generations, move next population into place as the current popoulation.
    /// 3. Handle any data-related updates including systematics and files that need to be printed.
    /// 4. Increment the current update number.
    void Update();

    /// Run the Process member function on all organisms in the population; forward any args passed
    /// into this function.
    template <typename... ARGS>
    void Process(ARGS &&... args) {   // Redirect to all orgs in the population!
      for (Ptr<ORG> org : pop) { if (org) org->Process(args...); }
    }

    /// Run the Process member function on a single, specified organism in the population;
    /// forward any args passed into this function.
    template <typename... ARGS>
    void ProcessID(size_t id, ARGS &&... args) {   // Redirect to all orgs in the population!
      if (pop[id]) pop[id]->Process(std::forward<ARGS>(args)...);
    }

    /// Reset the hardware for all organisms.
    void ResetHardware() {
      for (Ptr<ORG> org : pop) { if (org) org->ResetHardware(); }
    }

    // --- CALCULATE FITNESS ---

    /// Use the configured fitness function on the specified organism.
    double CalcFitnessOrg(ORG & org);

    /// Use the configured fitness function on the organism at the specified position.
    double CalcFitnessID(size_t id);

    /// Calculate the fitness of all organisms, storing the results in the cache.
    void CalcFitnessAll() const {
      emp_assert(cache_on, "Trying to calculate fitness of all orgs without caching.");
      for (size_t id = 0; id < pop.size(); id++) CalcFitnessID(id);
    }

    /// Turn on (or off) fitness caching for individual organisms.
    void SetCache(bool _in=true) { cache_on = _in; }

    /// Remove all currently cached fitness values (useful with changing environments, etc.)
    void ClearCache() { fit_cache.resize(0); }


    // --- MUTATIONS! ---

    /// Use mutation function on a single, specified organism.
    void DoMutationsOrg(ORG & org) {
      emp_assert(fun_do_mutations);  emp_assert(random_ptr);
      fun_do_mutations(org, *random_ptr);
    }

    /// Use mutation function on the organism at the specified position in the population.
    void DoMutationsID(size_t id) {
      emp_assert(pop[id]);
      DoMutationsOrg(*(pop[id]));
    }

    /// Use mutation function on ALL organisms in the population.
    void DoMutations(size_t start_id=0) {
      for (size_t id = start_id; id < pop.size(); id++) { if (pop[id]) DoMutationsID(id); }
    }

    // --- MANIPULATE ORGS IN POPULATION ---

    /// Remove all organisms from the world.
    void Clear();

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
      for (size_t i = new_size; i < pop.size(); i++) RemoveOrgAt(i); // Remove orgs past new size.
      pop.resize(new_size, nullptr);                                 // Default new orgs to null.
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
    void AddOrgAt(Ptr<ORG> new_org, WorldPosition pos, WorldPosition p_pos=WorldPosition());

    /// RemoveOrgAt is the core function to remove organisms from the population.
    /// Note: This function ignores population structure, so requires you to manage your own structure.
    void RemoveOrgAt(WorldPosition pos);

    /// Inject an organism using the default injection scheme.
    void Inject(const genome_t & mem, size_t copy_count=1);

    /// Inject an organism at a specific position.
    void InjectAt(const genome_t & mem, const WorldPosition pos);

    /// Inject a random organism (constructor must facilitate!)
    template <typename... ARGS> void InjectRandomOrg(ARGS &&... args);

    /// Place one or more copies of an offspring into population; return position of last placed.
    WorldPosition DoBirth(const genome_t & mem, size_t parent_pos, size_t copy_count=1);

    // Kill off organism at the specified position (same as RemoveOrgAt, but callable externally)
    void DoDeath(const WorldPosition pos) { RemoveOrgAt(pos); }

    // Kill off an organism using internal kill method setup by population structure.
    void DoDeath() { fun_kill_org(); }

    // --- RANDOM FUNCTIONS ---

    /// Return a reference to the random number generator currently being used by world.
    Random & GetRandom() { emp_assert(random_ptr); return *random_ptr; }

    /// Setup a new random number generator created elsewhere.
    void SetRandom(Random & r);

    /// Create a new random number generator (that World will manage)
    void NewRandom(int seed=-1);

    /// Get the position a cell, at random.
    size_t GetRandomCellID() { return GetRandom().GetUInt(pop.size()); }

    /// Get the position a cell in a range, at random.
    size_t GetRandomCellID(size_t min_id, size_t max_id) {
      emp_assert(min_id < max_id && max_id <= pop.size());
      return min_id + GetRandom().GetUInt(max_id - min_id);
    }

    /// Use the specified function to get a neighbor (if not set, assume well mixed).
    WorldPosition GetRandomNeighborPos(WorldPosition pos) { return fun_get_neighbor(pos); }

    /// Get the id of a random *occupied* cell.
    size_t GetRandomOrgID();

    /// Get an organism from a random occupied cell.
    ORG & GetRandomOrg() { return *pop[GetRandomOrgID()]; }

    // --- POPULATION ANALYSIS ---

    /// Find ALL cell IDs that return true in the provided filter.
    emp::vector<size_t> FindCellIDs(const std::function<bool(ORG*)> & filter);

    // Simple techniques for using FindCellIDs()

    /// Return IDs of all occupied cells in the population.
    emp::vector<size_t> GetValidOrgIDs() { return FindCellIDs([](ORG*org){ return (bool) org; }); }

    /// Return IDs of all empty cells in the population.
    emp::vector<size_t> GetEmptyPopIDs() { return FindCellIDs([](ORG*org){ return !org; }); }


    // --- POPULATION MANIPULATIONS ---

    /// Run population through a bottleneck to (potentially) shrink it.
    void DoBottleneck(const size_t new_size, bool choose_random=true);

    /// Perform a Serial Transfer where a fixed percentage of current organisms are maintained.
    void SerialTransfer(const double keep_frac);

    // --- PRINTING ---

    /// Print all organisms in the population using previously provided print function.
    void Print(std::ostream & os = std::cout, const std::string & empty="-", const std::string & spacer=" ");

    /// Print unique organisms and the number of copies of each that exist.
    void PrintOrgCounts(std::ostream & os = std::cout);

    /// Print the organisms layed out in a grid structure (assumes a grid population.)
    void PrintGrid(std::ostream& os=std::cout, const std::string & empty="-", const std::string & spacer=" ");


    // --- FOR VECTOR COMPATIBILITY ---

    /// [std::vector compatibility] How big is the world?
    size_t size() const { return pop.size(); }

    /// [std::vector compatibility] Update world size.
    void resize(size_t new_size) { Resize(new_size); }

    /// [std::vector compatibility] Remove all organisms.
    void clear() { Clear(); }

    /// [std::vector compatibility] Return iterator to first organism.
    iterator_t begin() { return iterator_t(this, 0); }

    /// [std::vector compatibility] Return iterator just past last organism.
    iterator_t end() { return iterator_t(this, (int) pop.size()); }

  };

  // =============================================================
  // ===                                                       ===
  // ===  Out-of-class member function definitions from above  ===
  // ===                                                       ===
  // =============================================================

  template <typename ORG>
  void World<ORG>::AddOrgAt(Ptr<ORG> new_org, WorldPosition pos, WorldPosition p_pos) {
    emp_assert(new_org);         // The new organism must exist.
    emp_assert(pos.IsValid());   // Position must be legal.

    // If new organism is going into the active population, trigger signal before doing so.
    if (pos.IsActive()) { before_placement_sig.Trigger(*new_org, pos.GetIndex()); }

    for (Ptr<SystematicsBase<ORG> > s : systematics) {
      s->SetNextParent((int) p_pos.GetIndex());
    }

    // Clear out any old organism at this position.
    RemoveOrgAt(pos);

    pops.MakeValid(pos);                 // Make sure we have room for new organism
    pops(pos) = new_org;                 // Put org into place.

    // Track org count
    if (pos.IsActive()) ++num_orgs;

    // Track the new systematics info
    for (Ptr<SystematicsBase<ORG> > s : systematics) {
      s->AddOrg(*new_org, (int) pos.GetIndex(), (int) update, !pos.IsActive());
    }

    SetupOrg(*new_org, pos, *random_ptr);

    // If new organism is in the active population, trigger associated signal.
    if (pos.IsActive()) { on_placement_sig.Trigger(pos.GetIndex()); }
  }

  template<typename ORG>
  void World<ORG>::RemoveOrgAt(WorldPosition pos) {
    size_t id = pos.GetIndex(); // Identify specific index.
    pop_t & cur_pop = pops[pos.GetPopID()];
    if (id >= cur_pop.size() || !cur_pop[id]) return; // Nothing to remove!
    if (pos.IsActive()) on_death_sig.Trigger(id);    // If active, signal that org is about to die.
    cur_pop[id].Delete();                            // Delete the organism...
    cur_pop[id] = nullptr;                           // ...and reset the pointer to null

    if (pos.IsActive()) {
      --num_orgs;                                    // Track one fewer organisms in the population
      if (cache_on) ClearCache(id);                  // Delete any cached info about this organism
      for (Ptr<SystematicsBase<ORG> > s : systematics) {
        s->RemoveOrg((int) pos.GetIndex());          // Notify systematics about organism removal
      }
    } else {
      for (Ptr<SystematicsBase<ORG> > s : systematics) {
        s->RemoveNextOrg((int) pos.GetIndex());      // Notify systematics about organism removal
      }
    }
  }

  template<typename ORG>
  void World<ORG>::SetPopStruct_Grow(bool synchronous_gen) {
    pop_sizes.resize(0);
    is_synchronous = synchronous_gen;
    is_space_structured = false;
    is_pheno_structured = false;

    // -- Setup functions --
    // Append at end of population
    fun_find_inject_pos = [this](Ptr<ORG> new_org) {
      (void) new_org;
      return pop.size();
    };

    // Neighbors are anywhere in the same population.
    fun_get_neighbor = [this](WorldPosition pos) { return pos.SetIndex(GetRandomCellID()); };

    // Kill random organisms and move end into vacant position to keep pop compact.
    fun_kill_org = [this](){
      const size_t last_id = pop.size() - 1;
      Swap(GetRandomCellID(), last_id);
      RemoveOrgAt(last_id);
      pop.resize(last_id);
      return last_id;
    };

    if (synchronous_gen) {
      // Append births into the next population.
      fun_find_birth_pos = [this](Ptr<ORG> new_org, WorldPosition parent_pos) {
        emp_assert(new_org);      // New organism must exist.
        return WorldPosition(pops[1].size(), 1);   // Append it to the NEXT population
      };

      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always append to current population.
      fun_find_birth_pos = [this](Ptr<ORG> new_org, WorldPosition parent_pos) {
        return WorldPosition(pop.size());
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Grow");
    SetSynchronousSystematics(synchronous_gen);
  }

  template<typename ORG>
  void World<ORG>::SetPopStruct_Mixed(bool synchronous_gen) {
    pop_sizes.resize(0);
    is_synchronous = synchronous_gen;
    is_space_structured = false;
    is_pheno_structured = false;

    // -- Setup functions --
    // Append at end of population
    fun_find_inject_pos = [this](Ptr<ORG> new_org) {
      (void) new_org;
      return pop.size();
    };

    // Neighbors are anywhere in the same population.
    fun_get_neighbor = [this](WorldPosition pos) { return pos.SetIndex(GetRandomCellID()); };

    // Kill random organisms and move end into vacant position to keep pop compact.
    fun_kill_org = [this](){
      const size_t kill_id = GetRandomCellID();
      RemoveOrgAt(kill_id);
      return kill_id;
    };

    if (synchronous_gen) {
      // Append births into the next population.
      fun_find_birth_pos = [this](Ptr<ORG> new_org, WorldPosition parent_id) {
        emp_assert(new_org);                        // New organism must exist.
        return WorldPosition(pops[1].size(), 1);   // Append it to the NEXT population
      };

      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neigbor in current population.
      fun_find_birth_pos = [this](Ptr<ORG> new_org, WorldPosition parent_id) {
        return WorldPosition(fun_get_neighbor(parent_id)); // Place org in existing population.
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Mixed");
    SetSynchronousSystematics(synchronous_gen);
  }

  template<typename ORG>
  void World<ORG>::SetPopStruct_Grid(size_t width, size_t height, bool synchronous_gen) {
    Resize(width, height);
    is_synchronous = synchronous_gen;
    is_space_structured = true;
    is_pheno_structured = false;

    // -- Setup functions --
    // Inject a random position in grid
    fun_find_inject_pos = [this](Ptr<ORG> new_org) {
      (void) new_org;
      return WorldPosition(GetRandomCellID());
    };

    // neighbors are in 9-sized neighborhood.
    fun_get_neighbor = [this](WorldPosition pos) {
      emp_assert(random_ptr);
      emp_assert(pop_sizes.size() == 2);
      const size_t size_x = pop_sizes[0];
      const size_t size_y = pop_sizes[1];
      const size_t id = pos.GetIndex();
      const int offset = random_ptr->GetInt(9);
      const int rand_x = (int) (id%size_x) + offset%3 - 1;
      const int rand_y = (int) (id/size_x) + offset/3 - 1;
      const auto neighbor_id = emp::Mod(rand_x, (int) size_x) + emp::Mod(rand_y, (int) size_y) * (int)size_x;
      return pos.SetIndex(neighbor_id);
    };

    fun_kill_org = [this](){
      const size_t kill_id = GetRandomCellID();
      RemoveOrgAt(kill_id);
      return kill_id;
    };

    if (synchronous_gen) {
      // Place births in a neighboring position in the new grid.
      fun_find_birth_pos = [this](Ptr<ORG> new_org, WorldPosition parent_pos) {
        emp_assert(new_org);                                    // New organism must exist.
        WorldPosition next_pos = fun_get_neighbor(parent_pos);  // Place near parent.
        return next_pos.SetPopID(1);                            // Adjust position to next pop and place..
      };
      SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neighbor in current population.
      fun_find_birth_pos = [this](Ptr<ORG> new_org, WorldPosition parent_pos) {
        return WorldPosition(fun_get_neighbor(parent_pos)); // Place org in existing population.
      };
      SetAttribute("SynchronousGen", "False");
    }

    SetAttribute("PopStruct", "Grid");
    SetSynchronousSystematics(synchronous_gen);
  }

  // Add a new data file constructed elsewhere.
  template<typename ORG>
  DataFile & World<ORG>::AddDataFile(emp::Ptr<DataFile> file) {
    size_t id = files.size();
    files.push_back(file);
    return *files[id];
  }


  // Create a new, arbitrary file.
  template<typename ORG>
  DataFile & World<ORG>::SetupFile(const std::string & filename) {
    size_t id = files.size();
    files.emplace_back();
    files[id].New(filename);
    return *files[id];
  }

  // A data file (default="fitness.csv") that contains information about the population's fitness.
  template<typename ORG>
  DataFile & World<ORG>::SetupFitnessFile(const std::string & filename, const bool & print_header) {
    auto & file = SetupFile(filename);
    auto node = GetFitnessDataNode();
    file.AddVar(update, "update", "Update");
    file.AddMean(*node, "mean_fitness", "Average organism fitness in current population.");
    file.AddMin(*node, "min_fitness", "Minimum organism fitness in current population.");
    file.AddMax(*node, "max_fitness", "Maximum organism fitness in current population.");
    file.AddInferiority(*node, "inferiority", "Average fitness / maximum fitness in current population.");
    if (print_header) file.PrintHeaderKeys();
    return file;
  }

  // A data file (default="systematics.csv") that contains information about the population's
  // phylogeny and lineages.
  template<typename ORG>
  DataFile & World<ORG>::SetupSystematicsFile(std::string label, const std::string & filename, const bool & print_header) {
    emp_assert(Has(systematics_labels, label), "Invalid systematics tracker requested.", label);
    return SetupSystematicsFile(systematics_labels[label], filename, print_header);
  }

  // A data file (default="systematics.csv") that contains information about the population's
  // phylogeny and lineages.
  template<typename ORG>
  DataFile & World<ORG>::SetupSystematicsFile(size_t id, const std::string & filename, const bool & print_header) {
    emp_assert(systematics.size() > 0, "Cannot track systematics file. No systematics file to track.");
    emp_assert(id < systematics.size(), "Invalid systematics file requested to be tracked.");
    auto & file = SetupFile(filename);
    file.AddVar(update, "update", "Update");
    file.template AddFun<size_t>( [this, id](){ return systematics[id]->GetNumActive(); }, "num_taxa", "Number of unique taxonomic groups currently active." );
    file.template AddFun<size_t>( [this, id](){ return systematics[id]->GetTotalOrgs(); }, "total_orgs", "Number of organisms tracked." );
    file.template AddFun<double>( [this, id](){ return systematics[id]->GetAveDepth(); }, "ave_depth", "Average Phylogenetic Depth of Organisms." );
    file.template AddFun<size_t>( [this, id](){ return systematics[id]->GetNumRoots(); }, "num_roots", "Number of independent roots for phlogenies." );
    file.template AddFun<int>(    [this, id](){ return systematics[id]->GetMRCADepth(); }, "mrca_depth", "Phylogenetic Depth of the Most Recent Common Ancestor (-1=none)." );
    file.template AddFun<double>( [this, id](){ return systematics[id]->CalcDiversity(); }, "diversity", "Genotypic Diversity (entropy of taxa in population)." );

    if (print_header) file.PrintHeaderKeys();
    return file;
  }

  // A data file (default="population.csv") contains information about the current population.
  template<typename ORG>
  DataFile & World<ORG>::SetupPopulationFile(const std::string & filename, const bool & print_header) {
    auto & file = SetupFile(filename);
    file.AddVar(update, "update", "Update");
    file.template AddFun<size_t>( [this](){ return GetNumOrgs(); }, "num_orgs", "Number of organisms currently living in the population." );
    if (print_header) file.PrintHeaderKeys();
    return file;
  }

  template<typename ORG>
  void World<ORG>::SetSharedFitFun(const fun_calc_fitness_t & fit_fun,
                                   const fun_calc_dist_t & dist_fun,
                                   double sharing_threshold, double alpha)
  {
    fun_calc_fitness = [this, fit_fun, dist_fun, sharing_threshold, alpha](ORG & org) {
      double niche_count = 0;
      for (Ptr<ORG> org2 : pop) {
        if (!org2) continue;
        double dist = dist_fun(org, *org2);
        niche_count += std::max(1.0 - std::pow(dist/sharing_threshold, alpha), 0.0);
      }
      return fit_fun(org)/niche_count;
    };
  }

  // --- Updating the world! ---

  template<typename ORG>
  void World<ORG>::Update() {
    // 1. Send out an update signal for any external functions to trigger.
    on_update_sig.Trigger(update);

    // 2. If synchronous generations (i.e, pops[1] is not empty), move next population into
    //    place as the current popoulation.
    if (pops[1].size()) {
      // Trigger signals for orgs in next pop before they are moved into the active pop.
      for (size_t i = 0; i < pops[1].size(); i++) {
        if (!pops[1][i]) continue;
        before_placement_sig.Trigger(*pops[1][i], i);  // Trigger that org is about to be placed.
      }

      // Clear out current pop.
      for (size_t i = 0; i < pop.size(); i++) RemoveOrgAt(i);
      pop.resize(0);
      std::swap(pops[0], pops[1]);            // Move next pop into place.

      // Update the active population.
      num_orgs = 0;
      for (size_t i = 0; i < pop.size(); i++) {
        if (!pop[i]) continue;        // Ignore empty positions.
        ++num_orgs;                   // Keep count of number of organisms
        on_placement_sig.Trigger(i);  // Trigger that organism has been placed.
      }
    }

    // 3. Handle systematics and any data files that need to be printed this update.

    // Tell systematics manager to swap next population and population
    for (Ptr<SystematicsBase<ORG>> s : systematics) {
      s->Update();
    }

    for (auto file : files) file->Update(update);

    // 4. Increment the current update number; i.e., count calls to Update().
    update++;
  }

  template<typename ORG>
  double World<ORG>::CalcFitnessOrg(ORG & org) {
    emp_assert(fun_calc_fitness);
    return fun_calc_fitness(org);
  }

  template<typename ORG>
  double World<ORG>::CalcFitnessID(size_t id) {
    if (!pop[id]) return 0.0;
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
    for (size_t pop_id = 0; pop_id < 2; pop_id++) {
      for (size_t i = 0; i < pops[pop_id].size(); i++) RemoveOrgAt(WorldPosition(i,pop_id));
      pops[pop_id].resize(0);
    }
  }

  template <typename ORG>
  void World<ORG>::Inject(const genome_t & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      inject_ready_sig.Trigger(*new_org);
      const WorldPosition pos = fun_find_inject_pos(new_org);

      if (pos.IsValid()) AddOrgAt(new_org, pos);  // If placement position is valid, do so!
      else new_org.Delete();                      // Otherwise delete the organism.
    }
  }

  template <typename ORG>
  void World<ORG>::InjectAt(const genome_t & mem, const WorldPosition pos) {
    emp_assert(pos.IsValid());
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    inject_ready_sig.Trigger(*new_org);
    AddOrgAt(new_org, pos);
  }

  template <typename ORG>
  template <typename... ARGS>
  void World<ORG>::InjectRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InjectRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    inject_ready_sig.Trigger(*new_org);
    const WorldPosition pos = fun_find_inject_pos(new_org);

    if (pos.IsValid()) AddOrgAt(new_org, pos);  // If placement position is valid, do so!
    else new_org.Delete();                      // Otherwise delete the organism.
  }

  // Give birth to (potentially) multiple offspring; return position of last placed.
  // Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
  // Additional signal triggers occur in AddOrgAt.
  template <typename ORG>
  WorldPosition World<ORG>::DoBirth(const genome_t & mem, size_t parent_pos, size_t copy_count) {
    before_repro_sig.Trigger(parent_pos);
    WorldPosition pos;                                        // Position of each offspring placed.
    for (size_t i = 0; i < copy_count; i++) {                 // Loop through offspring, adding each
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      offspring_ready_sig.Trigger(*new_org, parent_pos);
      pos = fun_find_birth_pos(new_org, parent_pos);

      if (pos.IsValid()) AddOrgAt(new_org, pos, parent_pos);  // If placement pos is valid, do so!
      else new_org.Delete();                                  // Otherwise delete the organism.
    }
    return pos;
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

  // Run population through a bottleneck to (probably) shrink it.
  template<typename ORG>
  void World<ORG>::DoBottleneck(const size_t new_size, bool choose_random) {
    if (new_size >= num_orgs) return;  // No bottleneck needed!

    if (is_space_structured || is_pheno_structured) {
      // @CAO: Need to implement bottlenecks for structured populations.
      emp_assert(false, "Not implemented yet.");
    } else {
      // If we are supposed to keep only random organisms, shuffle the beginning into place!
      if (choose_random) {
        for (size_t to = 0; to < new_size; to++) {
          const size_t from = random_ptr->GetUInt(to, pop.size());
          if (from == to) continue;
          Swap(to, from);
        }
      }

      // Clear out all of the organisms we are removing and shrink the population.
      for (size_t i = new_size; i < pop.size(); ++i) RemoveOrgAt(i);
      pop.resize(new_size);
      ClearCache();
    }
  }

  template<typename ORG>
  void World<ORG>::SerialTransfer(const double keep_frac) {
    emp_assert(keep_frac >= 0.0 && keep_frac <= 1.0, keep_frac);

    // For a structured population, test position-by-position.
    if (is_space_structured || is_pheno_structured) {
      // Loop over the current population to clear out anyone who fails to be transferred.
      const double remove_frac = 1.0 - keep_frac;
      for (size_t i = 0; i < pop.size(); ++i) {
        if (random_ptr->P(remove_frac)) {
          RemoveOrgAt(i);
          ClearCache(i);
        }
      }
    }

    // For an unstructured population, keep all living organisms at the beginning.
    else {
      size_t live_pos = 0;
      for (size_t test_pos = 0; test_pos < pop.size(); test_pos++) {
        // If this organism is kept, keep it compact with the others.
        if (random_ptr->P(keep_frac)) {
          Swap(live_pos, test_pos);
          live_pos++;
        }

        // This organism didn't make the cut; kill it.
        else RemoveOrgAt(test_pos);
      }

      // Reflect the new population size.
      pop.resize(live_pos);
      ClearCache();
    }
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
    emp_assert(pop_sizes.size() == 2);
    const size_t size_x = pop_sizes[0];
    const size_t size_y = pop_sizes[1];
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

}

#endif
