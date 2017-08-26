//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

/**
 *  @file  World.h
 *  @brief Definition of a base class for a World template for use in evolutionary algorithms.
 *
 *  A definition of the emp::World template, linking in specialized file handling, iterators,
 *  and selection techniques for evolutionary computation applications.
 */

//  Developer Notes:
//  * Mutations are currently not interacting properly with lineages.  Organisms are born, added
//    to the lineage and THEN mutated.  Ideally, all mutations should occur on birth (via a
//    signal?), but we have to make sure things like elite selection can still turn off mutations.
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
#include "../control/Signal.h"
#include "../control/SignalControl.h"
#include "../meta/reflection.h"
#include "../tools/map_utils.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"
#include "../tools/string_utils.h"

// World-specific includes.
#include "Systematics.h"    // Track relationships among organisms.
#include "World_file.h"     // Helper to determine when specific events should occur.
#include "World_iterator.h" // Allow iteration through organisms in a world.
#include "World_reflect.h"  // Handle needed reflection on incoming organism classes.
#include "World_select.h"   // Include all built-in selection functions for World.

namespace emp {

  ///  Setup a World with a population of organisms that can evolve or deal with ecological effects.
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
  ///   0. If you set the fitness function using SetFitFun(), it will have priority.
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
  ///   1. Org Print() member function
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
    using iterator_t = World_iterator<this_t>; ///< Type for this world's iterators.

    using genome_t = typename emp::find_genome_t<ORG>;  ///< Type of underlying genomes.
    using genotype_t = emp::Taxon<genome_t>;            ///< Type of full genome category.

    /// Function type for calculating fitness.
    using fun_calc_fitness_t = std::function<double(ORG&)>;

    /// Function type for calculating the distance between two organisms.
    using fun_calc_dist_t    = std::function<double(ORG&,ORG&)>;

    /// Function type for a mutation operator on an organisms.
    using fun_do_mutations_t = std::function<size_t(ORG&,Random&)>;

    /// Function type for printing an organism's info to an output stream.
    using fun_print_org_t    = std::function<void(ORG&,std::ostream &)>;

    /// Function type for retrieving a genome from an organism.
    using fun_get_genome_t   = std::function<const genome_t & (ORG &)>;

    /// Function type for injecting organisms into a world
    using fun_add_inject_t   = std::function<size_t(Ptr<ORG>)>;

    /// Function type for adding a newly born organism into a world.
    using fun_add_birth_t    = std::function<size_t(Ptr<ORG>, size_t)>;

    /// Function type for identifying an organism's random neighbor.
    using fun_get_neighbor_t = std::function<size_t(size_t)>;

  protected:
    // Internal state member variables
    Ptr<Random> random_ptr;         ///< @brief Random object to use.
    bool random_owner;              ///< Did we create our own random number generator?
    emp::vector<Ptr<ORG>> pop;      ///< All of the spots in the population.
    emp::vector<Ptr<ORG>> next_pop; ///< Population being setup for next generation.
    size_t num_orgs;                ///< How many organisms are actually in the population.
    size_t update;                  ///< How many times has the Update() method been called?
    emp::vector<double> fit_cache;  ///< vector size == 0 when not caching; uncached values == 0.
    emp::vector<Ptr<genotype_t>> genotypes;      ///< Genotypes for the corresponding orgs.
    emp::vector<Ptr<genotype_t>> next_genotypes; ///< Genotypes for corresponding orgs in next_pop.

    // Configuration settings
    std::string name;               ///< Name of this world (for use in configuration.)
    bool cache_on;                  ///< Should we be caching fitness values?
    size_t size_x;                  ///< If a grid, track width; if pools, track pool size
    size_t size_y;                  ///< If a grid, track height; if pools, track num pools.
    emp::vector<World_file> files;  ///< Output files.

    bool is_synchronous;            ///< Does this world have synchronous generations?
    bool is_structured;             ///< Do we have any structured population? (false=well mixed)

    /// Potential data nodes -- these should be activated only if in use.
    Ptr<DataMonitor<double>> data_node_fitness;

    // Configurable functions.
    fun_calc_fitness_t  fun_calc_fitness;   ///< Function to evaluate fitness for provided organism.
    fun_do_mutations_t  fun_do_mutations;   ///< Function to mutate an organism.
    fun_print_org_t     fun_print_org;      ///< Function to print an organism.
    fun_get_genome_t    fun_get_genome;     ///< Determine the genome object of an organism.
    fun_add_inject_t    fun_add_inject;     ///< Technique to inject a new, external organism.
    fun_add_birth_t     fun_add_birth;      ///< Technique to add a new offspring organism.
    fun_get_neighbor_t  fun_get_neighbor;   ///< Choose a random neighbor near specified id.

    /// Attributes are a dynamic way to track extra characteristics about a world.
    std::map<std::string, std::string> attributes;

    /// Phylogeny and line-of-descent data collection.
    Systematics<genome_t> systematics;

    // == Signals ==
    SignalControl control;  // Setup the world to control various signals.
    Signal<void(size_t)> before_repro_sig;    ///< Trigger signal before organism gives birth.
    Signal<void(ORG &)> offspring_ready_sig;  ///< Trigger signal when offspring organism is built.
    Signal<void(ORG &)> inject_ready_sig;     ///< Trigger when external organism is ready to inject.
    Signal<void(size_t)> org_placement_sig;   ///< Trigger when any organism is placed into world.
    Signal<void(size_t)> on_update_sig;       ///< Trigger at the beginning of Update()
    Signal<void(size_t)> on_death_sig;        ///< Trigger when any organism dies.

    /// AddOrgAt is the only way to add organisms to active population (others must go through here)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype=nullptr);

    /// AddNextOrgAt build up the next population during synchronous generations.
    size_t AddNextOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype=nullptr);

    /// RemoveOrgAt is the only way to remove an active organism.
    void RemoveOrgAt(size_t pos);

    /// RemoveNextOrgAt removes an organism waiting to placed into the next generation.
    void RemoveNextOrgAt(size_t pos);

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
    /// If no random number generator is provided, one is created within the world.
    /// If no name is provided, the world remains nameless.
    World(Ptr<Random> rnd=nullptr, std::string _name="")
      : random_ptr(rnd), random_owner(false), pop(), next_pop(), num_orgs(0), fit_cache()
      , genotypes(), next_genotypes()
      , name(_name), cache_on(false), size_x(0), size_y(0), files()
      , is_synchronous(false), is_structured(false)
      , data_node_fitness(nullptr)
      , fun_calc_fitness(), fun_do_mutations(), fun_print_org(), fun_get_genome()
      , fun_add_inject(), fun_add_birth(), fun_get_neighbor()
      , attributes(), systematics(true,true,true)
      , control()
      , before_repro_sig(to_string(name,"::before-repro"), control)
      , offspring_ready_sig(to_string(name,"::offspring-ready"), control)
      , inject_ready_sig(to_string(name,"::inject-ready"), control)
      , org_placement_sig(to_string(name,"::org-placement"), control)
      , on_update_sig(to_string(name,"::on-update"), control)
      , on_death_sig(to_string(name,"::on-death"), control)
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
      if (data_node_fitness) data_node_fitness.Delete();
    }

    // --- Accessing Organisms or info ---

    /// How many organisms can fit in the world?
    size_t GetSize() const { return pop.size(); }

    /// How many organisms are currently in the world?
    size_t GetNumOrgs() const { return num_orgs; }

    /// What update number is the world currently on? (assumes Update() is being used)
    size_t GetUpdate() const { return update; }

    /// How many cells wide is the world? (assumes grids are active.)
    size_t GetWidth() const { return size_x; }

    /// How many cells tall is the world? (assumes grids are active.)
    size_t GetHeight() const { return size_y; }

    /// Does the specified cell ID have an organism in it?
    bool IsOccupied(size_t i) const { return pop[i] != nullptr; }

    /// Are we currently caching fitness values?
    bool IsCacheOn() const { return cache_on; }

    /// Are generations being evaluated synchronously?
    /// (i.e., Update() places all births into the population after removing all current organisms.)
    bool IsSynchronous() const { return is_synchronous; }

    /// Is there some sort of structure to the population?
    /// (i.e., are some organisms closer together than others; false implies "well-mixed".)
    bool IsStructured() const { return is_structured; }

    /// Index into a world to obtain a const reference to an organism.  Any manipulations to
    /// organisms should go through other functions to be tracked appropriately.
    /// Will trip assert if cell is not occupied.
    const ORG & operator[](size_t id) const {
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pop[id]);
    }

    /// Retrieve a const reference to the organsim as the specified position.
    /// Same as operator[]; will trip assert if cell is not occupied.
    ORG & GetOrg(size_t id) {
      emp_assert(pop[id] != nullptr, id);  // Should not index to a null organism!
      return *(pop[id]);
    }

    /// Retrieve a const reference to the organsim as the specified x,y coordinates.
    /// (currently used only in a grid world)
    ORG & GetOrg(size_t x, size_t y) { return GetOrg(x+y*size_x); }

    /// Retrive a pointer to the contents of a speciefied cell; will be nullptr if the cell is
    /// not occupied.
    const Ptr<ORG> GetOrgPtr(size_t id) const { return pop[id]; }

    /// Retrieve the genome corresponding to a specified organism.
    const genome_t & GetGenome(ORG & org) { return fun_get_genome(org); }

    /// Retrive the genome corresponding to the organism at the specified position.
    const genome_t & GetGenomeAt(size_t id) { return fun_get_genome(GetOrg(id)); }

    /// Get the systematics manager (which is tracking lineages in the population.)
    const Systematics<genome_t> & GetSystematics() const { return systematics; }

    /// Print the full line-of-descent to the organism at the specified position in the popoulation.
    void PrintLineage(size_t id, std::ostream & os=std::cout) const {
      systematics.PrintLineage(genotypes[id], os);
    }

    // --- CONFIGURE ---

    /// Set the population to be well-mixed (with all organisms counting as neighbors.)
    /// Argument determines if the generations should be synchronous (true) or not (false, default)
    void SetWellMixed(bool synchronous_gen=false);

    /// Set the population to be a grid of cells using the specified dimensions.  The third
    /// argument determines if the generations should be synchronous (true) or not (false, default)
    void SetGrid(size_t width, size_t height, bool synchronous_gen=false);

    /// Set the population to be a set of pools that are individually well mixed, but with limited
    /// migtation.  Arguments are the number of pools, the size of each pool, and whether the
    /// generations should be synchronous (true) or not (false, default).
    void SetPools(size_t num_pools, size_t pool_size, bool synchronous_gen=false);

    /// Access a data node that tracks fitness information in the population.  The fitness will not
    /// be collected until the first Update() after this function is initially called, signaling
    /// the need for this information.
    DataMonitor<double> & GetFitnessDataNode() {
      if (!data_node_fitness) {
        data_node_fitness.New();
        // Collect fitnesses each update...
        OnUpdate(
          [this](size_t){
            data_node_fitness->Reset();
            for (size_t i = 0; i < pop.size(); i++) {
              if (IsOccupied(i)) data_node_fitness->AddDatum( CalcFitnessID(i) );
            }
          }
        );
      }
      return *data_node_fitness;
    }

    /// Setup an arbitrary file; no default filename available.
    World_file & SetupFile(const std::string & filename);

    /// Setup a file to be printed that collects fitness information over time.
    World_file & SetupFitnessFile(const std::string & filename="fitness.csv");

    /// Setup a file to be printed that collects systematics information over time.
    World_file & SetupSystematicsFile(const std::string & filename="systematics.csv");

    /// Setup a file to be printed that collects population information over time.
    World_file & SetupPopulationFile(const std::string & filename="population.csv");

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
    void SetGetGenomeFun(const fun_get_genome_t & gen_fun) { fun_get_genome = gen_fun; }

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
    SignalKey OnBeforeRepro(const std::function<void(size_t)> & fun) { return before_repro_sig.AddAction(fun); }

    /// Provide a function for World to call after an offspring organism has been created, but
    /// before it is inserted into the World.
    /// Trigger:  Offspring about to enter population
    /// Argument: Reference to organism about to be placed in population.
    /// Return:   Key value needed to make future modifications.
    SignalKey OnOffspringReady(const std::function<void(ORG &)> & fun) { return offspring_ready_sig.AddAction(fun); }

    /// Provide a function for World to call before an external organim is injected into the World.
    /// Trigger:  New organism about to be added to population from outside
    /// Argument: Reference to organism about to be placed in population.
    /// Return:   Key value needed to make future modifications.
    SignalKey OnInjectReady(const std::function<void(ORG &)> & fun) { return inject_ready_sig.AddAction(fun); }

    /// Provide a function for World to call immediately after any organism has been added.
    /// Trigger:  Organism has been added to population (either born or injected)
    /// Argument: Position of organism placed in the population.
    /// Return:   Key value needed to make future modifications.
    SignalKey OnOrgPlacement(const std::function<void(size_t)> & fun) { return org_placement_sig.AddAction(fun); }

    /// Privide a function for World to call each time Update() is run.
    /// Trigger:  New update is starting
    /// Argument: Update number (sequentially increasing)
    /// Return:   Key value needed to make future modifications.
    SignalKey OnUpdate(const std::function<void(size_t)> & fun) { return on_update_sig.AddAction(fun); }

    /// Privide a function for World to call each time an organism is about to die.
    /// Trigger:  Organism is about to be killed
    /// Argument: Position of organism about to die
    /// Return:   Key value needed to make future modifications.
    SignalKey OnOrgDeath(const std::function<void(size_t)> & fun) { return on_death_sig.AddAction(fun); }


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
    /// 3. Handle any data files that need to be printed this update.
    /// 4. Increment the current update number.
    void Update();

    /// Run the Process member function on all organisms in the population; forward any args passed
    /// into this function.
    template <typename... ARGS>
    void Process(ARGS &&... args) {   // Redirect to all orgs in the population!
      for (Ptr<ORG> org : pop) { if (org) org->Process(std::forward<ARGS>(args)...); }
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

    /// Change the size of the world.  If the new size is smaller than the old, remove any
    /// organisms outside the new range.  If larger, new positions are empty.
    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pop.size(); i++) RemoveOrgAt(i); // Remove orgs past new size.
      pop.resize(new_size, nullptr);                                 // Default new orgs to null.
    }

    /// Inject an organism using the default injection scheme.
    void Inject(const ORG & mem, size_t copy_count=1);

    /// Inject an organism at a specific position.
    void InjectAt(const ORG & mem, const size_t pos);

    /// Inject a random organism (constructor must facilitate!)
    template <typename... ARGS> void InjectRandomOrg(ARGS &&... args);

    /// Place a newborn organism into the population, by default rules and with parent information.
    size_t DoBirth(const ORG mem, size_t parent_pos);

    /// Place multiple copies of a newborn organism into the population.
    void DoBirth(const ORG mem, size_t parent_pos, size_t copy_count);

    // Kill off organism at the specified position (same as RemoveOrgAt, but callable externally)
    void DoDeath(const size_t pos) { RemoveOrgAt(pos); }

    // --- RANDOM FUNCTIONS ---

    /// Return a reference to the random number generator currently being used by world.
    Random & GetRandom() { return *random_ptr; }

    /// Setup a new random number generator created elsewhere.
    void SetRandom(Random & r);

    /// Create a new random number generator (that World will manage)
    void NewRandom(int seed=-1);

    /// Get the position any cell, at random.
    size_t GetRandomCellID() { return random_ptr->GetUInt(pop.size()); }

    /// Use the specified function to get a neighbor (if not set, assume well mixed).
    size_t GetRandomNeighborID(size_t id) { return fun_get_neighbor(id); }

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
  size_t World<ORG>::AddOrgAt(Ptr<ORG> new_org, size_t pos, Ptr<genotype_t> p_genotype) {
    emp_assert(new_org, pos);                            // The new organism must exist.

    // Determine new organism's genotype.
    Ptr<genotype_t> new_genotype = systematics.AddOrg(GetGenome(*new_org), p_genotype);
    if (pop.size() <= pos) pop.resize(pos+1, nullptr);  // Make sure we have room.
    RemoveOrgAt(pos);                                   // Clear out any old org.
    pop[pos] = new_org;                                 // Place new org.
    ++num_orgs;                                         // Track number of orgs.

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
    RemoveNextOrgAt(pos);                                          // Clear out any old org.
    next_pop[pos] = new_org;                                       // Place new org.

    // Track the new genotype.
    if (next_genotypes.size() <= pos) next_genotypes.resize(pos+1, nullptr);  // Make sure we fit genotypes.
    next_genotypes[pos] = new_genotype;

    return pos;
  }

  template<typename ORG>
  void World<ORG>::RemoveOrgAt(size_t pos) {
    if (!pop[pos]) return;                   // Nothing to remove!
    on_death_sig.Trigger(pos);               // Identify that this position is about to be removed
    pop[pos].Delete();                       // Delete the organism...
    pop[pos] = nullptr;                      // ...and reset the pointer to null
    --num_orgs;                              // Track one fewer organisms in the population
     if (cache_on) ClearCache(pos);          // Delete any cached info about this organism
    systematics.RemoveOrg( genotypes[pos] ); // Notify systematics about organism removal
    genotypes[pos] = nullptr;                // No longer track a genotype at this position
  }

  template<typename ORG>
  void World<ORG>::RemoveNextOrgAt(size_t pos) {
    if (!next_pop[pos]) return;                   // Nothing to remove!
    next_pop[pos].Delete();                       // Delete the organism...
    next_pop[pos] = nullptr;                      // ..and reset the pointer to null
    systematics.RemoveOrg( next_genotypes[pos] ); // Notify systematics manager about removal
    next_genotypes[pos] = nullptr;                // No longer track a genotype at this position
  }

  template<typename ORG>
  void World<ORG>::SetWellMixed(bool synchronous_gen) {
    size_x = 0; size_y = 0;
    is_synchronous = synchronous_gen;
    is_structured = false;

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
    is_synchronous = synchronous_gen;
    is_structured = true;

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
    is_synchronous = synchronous_gen;
    is_structured = true;

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

  // A new, arbitrary file.
  template<typename ORG>
  World_file & World<ORG>::SetupFile(const std::string & filename) {
    size_t id = files.size();
    files.emplace_back(filename);
    return files[id];
  }

  // A data file (default="fitness.csv") that contains information about the population's fitness.
  template<typename ORG>
  World_file & World<ORG>::SetupFitnessFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    auto & node = GetFitnessDataNode();
    file.AddVar(update, "update", "Update");
    file.AddMean(node, "mean_fitness", "Average organism fitness in current population.");
    file.AddMin(node, "min_fitness", "Minimum organism fitness in current population.");
    file.AddMax(node, "max_fitness", "Maximum organism fitness in current population.");
    file.AddInferiority(node, "inferiority", "Average fitness / maximum fitness in current population.");
    file.PrintHeaderKeys();
    return file;
  }

  // A data file (default="systematics.csv") that contains information about the population's
  // phylogeny and lineages.
  template<typename ORG>
  World_file & World<ORG>::SetupSystematicsFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    file.AddVar(update, "update", "Update");
    file.template AddFun<size_t>( [this](){ return systematics.GetNumActive(); }, "num_genotypes", "Number of unique genotype groups currently active." );
    file.template AddFun<size_t>( [this](){ return systematics.GetTotalOrgs(); }, "total_orgs", "Number of organisms tracked." );
    file.template AddFun<double>( [this](){ return systematics.GetAveDepth(); }, "ave_depth", "Average Phylogenetic Depth of Organisms." );
    file.template AddFun<size_t>( [this](){ return systematics.GetNumRoots(); }, "num_roots", "Number of independent roots for phlogenies." );
    file.template AddFun<int>( [this](){ return systematics.GetMRCADepth(); }, "mrca_depth", "Phylogenetic Depth of the Most Recent Common Ancestor (-1=none)." );
    file.template AddFun<double>( [this](){ return systematics.CalcDiversity(); }, "diversity", "Genotypic Diversity (entropy of genotypes in population)." );
    file.PrintHeaderKeys();
    return file;
  }

  // A data file (default="population.csv") contains information about the current population.
  template<typename ORG>
  World_file & World<ORG>::SetupPopulationFile(const std::string & filename) {
    auto & file = SetupFile(filename);
    file.AddVar(update, "update", "Update");
    file.template AddFun<size_t>( [this](){ return GetNumOrgs(); }, "num_orgs", "Number of organisms currently living in the population." );
    file.PrintHeaderKeys();
    return file;
  }

  template<typename ORG>
  void World<ORG>::SetSharedFitFun(const fun_calc_fitness_t & fit_fun,
                                   const fun_calc_dist_t & dist_fun,
                                   double sharing_threshold, double alpha)
  {
    fun_calc_fitness = [this, fit_fun, dist_fun, sharing_threshold, alpha](ORG & org) {
      double niche_count = 0;
      for (ORG * org2 : pop) {
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

    // 2. If synchronous generationsm (i.e, next_pop is not empty), move next population into
    //    place as the current popoulation.
    if (next_pop.size()) {
      // Clear out current pop.
      for (size_t i = 0; i < pop.size(); i++) RemoveOrgAt(i);
      pop.resize(0);

      std::swap(pop, next_pop);               // Move next_pop into place.
      std::swap(genotypes, next_genotypes);   // Move next_genotypes into place.
      next_genotypes.resize(0);               // Clear out genotype names.

      // Update the organism count.
      num_orgs = 0;
      for (size_t i = 0; i < pop.size(); i++) if (pop[i]) ++num_orgs;
    }

    // 3. Handle any data files that need to be printed this update.
    for (auto & file : files) file.Update(update);

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
    for (size_t i = 0; i < pop.size(); i++) RemoveOrgAt(i);
    for (size_t i = 0; i < next_pop.size(); i++) RemoveNextOrgAt(i);
    pop.resize(0);
    next_pop.resize(0);
  }

  template <typename ORG>
  void World<ORG>::Inject(const ORG & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      inject_ready_sig.Trigger(*new_org);
      const size_t pos = fun_add_inject(new_org);
      //SetupOrg(*new_org, &callbacks, pos);
      org_placement_sig.Trigger(pos);
    }
  }

  template <typename ORG>
  void World<ORG>::InjectAt(const ORG & mem, const size_t pos) {
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    inject_ready_sig.Trigger(*new_org);
    AddOrgAt(new_org, pos);
    org_placement_sig.Trigger(pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG>
  template <typename... ARGS>
  void World<ORG>::InjectRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InjectRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    inject_ready_sig.Trigger(*new_org);
    const size_t pos = fun_add_inject(new_org);
    org_placement_sig.Trigger(pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  // Give birth to a single offspring; return offspring position.
  template <typename ORG>
  size_t World<ORG>::DoBirth(const ORG mem, size_t parent_pos) {
    before_repro_sig.Trigger(parent_pos);
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    offspring_ready_sig.Trigger(*new_org);
    const size_t pos = fun_add_birth(new_org, parent_pos);
    org_placement_sig.Trigger(pos);
    // SetupOrg(*new_org, &callbacks, pos);
    return pos;
  }

  // Give birth to (potentially) multiple offspring; no return, but triggers can be tracked.
  template <typename ORG>
  void World<ORG>::DoBirth(const ORG mem, size_t parent_pos, size_t copy_count) {
    before_repro_sig.Trigger(parent_pos);
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      offspring_ready_sig.Trigger(*new_org);
      const size_t pos = fun_add_birth(new_org, parent_pos);
      org_placement_sig.Trigger(pos);
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
    for (size_t i = new_size; i < pop.size(); ++i) RemoveOrgAt(i);
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

}

#endif
