/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file Systematics.hpp
 *  @brief Track genotypes, species, clades, or lineages of organisms in a world.
 *
 *
 *  @todo Technically, we don't need to keep the ancestors in a set in order to track a lineage...
 *        If we delete all of their descendants they should automaticaly be deleted.
 *  @todo We should provide an option to back up systematics data to a file so that it doesn't all
 *        need to be kept in memory, especially if we're only doing post-analysis.
 *  @todo This inheritance system makes adding new systematics-related data tracking kind of a pain.
 *        Over time, this will probably become a maintainability problem. We could make the inheritance
 *        go away and just use signals, but then the World could not maintain systematics managers.
 * @todo This does not currently handle situations where organisms change locations during their
 *       lifetimes gracefully.
 */

#ifndef EMP_EVOLVE_SYSTEMATICS_HPP_INCLUDE
#define EMP_EVOLVE_SYSTEMATICS_HPP_INCLUDE

#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <ostream>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>

#include "../base/Ptr.hpp"
#include "../control/Signal.hpp"
#include "../data/DataFile.hpp"
#include "../data/DataManager.hpp"
#include "../data/DataNode.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/set_utils.hpp"
#include "../io/File.hpp"
#include "../math/info_theory.hpp"
#include "../math/stats.hpp"
#include "../tools/string_utils.hpp"

#include "SystematicsAnalysis.hpp"
#include "World_structure.hpp"

namespace emp {

  template <typename A, typename B, typename C>
  class Systematics;

  /// The systematics manager allows an optional second template type that
  /// can store additional data about each taxon in the phylogeny. Here are
  /// some structs containing common pieces of additional data to track.
  /// Note: You are responsible for filling these in! Adding the template
  /// just gives you a place to store your data.

  namespace datastruct {
    struct no_data {
        using has_fitness_t = std::false_type;
        using has_mutations_t = std::false_type;
        using has_phen_t = std::false_type;
    }; /// The default - an empty struct

    struct fitness {
        using has_fitness_t = std::true_type;
        using has_mutations_t = std::false_type;
        using has_phen_t = std::false_type;

        DataNode<double, data::Current, data::Range> fitness; /// This taxon's fitness (for assessing deleterious mutational steps)
        void RecordFitness(double fit) {
          fitness.Add(fit);
        }

        double GetFitness() const {
          return fitness.GetMean();
        }
    };

    /// Track information related to the mutational landscape
    /// Maps a string representing a type of mutation to a count representing
    /// the number of that type of mutation that occurred to bring about this taxon.
    template <typename PHEN_TYPE>
    struct mut_landscape_info {
      using phen_t = PHEN_TYPE;
      using has_phen_t = std::true_type;
      using has_mutations_t = std::true_type;
      using has_fitness_t = std::true_type;
      // using has_phenotype_t = true;

      std::unordered_map<std::string, int> mut_counts = {}; /// The number of mutations of each type that occurred to make this taxon
      DataNode<double, data::Current, data::Range> fitness; /// This taxon's fitness (for assessing deleterious mutational steps)
      PHEN_TYPE phenotype; /// This taxon's phenotype (for assessing phenotypic change)

      /// @returns this taxon's phenotype
      const PHEN_TYPE & GetPhenotype() const {
        return phenotype;
      }

      /// @returns this taxon's fitness
      double GetFitness() const {
        return fitness.GetMean();
      }

      /// Adds mutations to the list of mutations that occurred to make this taxon
      /// @param muts can contain as many strings (types of mutation) as desired, each accompanied
      /// by a number indicating how many of that mutation occurred
      /// Example: {"point_mutation":2, "insertion":1}
      void RecordMutation(std::unordered_map<std::string, int> & muts) {
        for (auto mut : muts) {
          if (Has(mut_counts, mut.first)) {
            mut_counts[mut.first] += mut.second;
          } else {
            mut_counts[mut.first] = mut.second;
          }
        }
      }

      /// Record the fitness of this taxon
      /// @param fit the fitness
      void RecordFitness(double fit) {
        fitness.Add(fit);
      }

      /// Record the phenotype of this taxon
      /// @param phen the phenotype
      void RecordPhenotype(PHEN_TYPE phen) {
        phenotype = phen;
      }

    };
  }

  /// @brief A Taxon represents a type of organism in a phylogeny.
  /// @param ORG_INFO The information type associated with an organism, used to categorize it.
  ///
  /// Genotypes are the most commonly used Taxon; in general taxa can be anything from a shared
  /// genome sequence, a phenotypic trait, or a even a position in the world (if you want to
  /// track an evolutionary pathway)
  template <typename ORG_INFO, typename DATA_STRUCT = emp::datastruct::no_data>
  class Taxon {
    template <typename A, typename B, typename C>
    friend class Systematics;
  protected:
    using this_t = Taxon<ORG_INFO, DATA_STRUCT>;
    using info_t = ORG_INFO;

    size_t id;                ///<  ID for this Taxon (Unique within this Systematics)
    const info_t info;        ///<  Details for the organisms associated within this taxanomic group.
    Ptr<this_t> parent;       ///<  Pointer to parent group (nullptr if injected)
    std::set<Ptr<this_t> > offspring; ///< Pointers to all immediate offspring taxa
    size_t num_orgs;          ///<  How many organisms currently exist of this group?
    size_t tot_orgs;          ///<  How many organisms have ever existed of this group?
    size_t num_offspring;     ///<  How many direct offspring groups exist from this one.
    size_t total_offspring;   ///<  How many total extant offspring taxa exist from this one (i.e. including indirect)
    size_t depth;             ///<  How deep in tree is this node? (Root is 0)
    double origination_time;  ///<  When did this taxon first appear in the population?
    double destruction_time;  ///<  When did this taxon leave the population?

    DATA_STRUCT data;         ///< A struct for storing additional information about this taxon

  public:
    using data_t = DATA_STRUCT;

    Taxon(size_t _id, const info_t & _info, Ptr<this_t> _parent=nullptr)
     : id (_id), info(_info), parent(_parent)
     , num_orgs(0), tot_orgs(0), num_offspring(0), total_offspring(0)
     , depth(parent ? (parent->depth+1) : 0)
     , destruction_time(std::numeric_limits<double>::infinity()) { ; }
    // Taxon(const Taxon &) = delete;
    Taxon(const Taxon &) = default; // TODO: Check with Charles about this
    Taxon(Taxon &&) = default;
    Taxon & operator=(const Taxon &) = delete;
    Taxon & operator=(Taxon &&) = default;

    bool operator<(const Taxon & other) const {
      return id < other.GetID();
    }

    /// Get a unique ID for this taxon; IDs are assigned sequentially, so newer taxa have higher IDs.
    size_t GetID() const { return id; }

    /// Retrieve the tracked info associated with this Taxon.
    const info_t & GetInfo() const { return info; }

    /// Retrieve a pointer to the parent Taxon.
    Ptr<this_t> GetParent() const { return parent; }
    void NullifyParent() {parent = nullptr;}

    /// Get the number of living organisms currently associated with this Taxon.
    size_t GetNumOrgs() const { return num_orgs; }

    /// Set the number of living organisms currently associated with this Taxon.
    void SetNumOrgs(size_t n) { num_orgs = n; }

    /// Get the total number of organisms that have ever lived associated with this Taxon
    size_t GetTotOrgs() const { return tot_orgs; }

    /// Set the total number of organisms that have ever lived associated with this Taxon
    void SetTotOrgs(size_t n) { tot_orgs = n; }

    /// Get the number of taxa that were produced by organisms from this Taxon.
    size_t GetNumOff() const { return num_offspring; }

    /// Get the number of taxanomic steps since the ancestral organism was injected into the World.
    size_t GetDepth() const { return depth; }

    /// Get data struct associated with this taxon
    data_t & GetData() {return data;}
    /// Get data struct associated with this taxon
    const data_t & GetData() const {return data;}

    /// Get pointers to this taxon's offspring
    std::set<Ptr<this_t> > GetOffspring() {return offspring;}

    /// Set this taxon's data struct to the given value
    void SetData(data_t d) {data = d;}

    /// @returns this taxon's origination time
    double GetOriginationTime() const {return origination_time;}
    /// Set this taxon's origination time
    void SetOriginationTime(double time) {origination_time = time;}

    /// @returns this taxon's destruction time
    double GetDestructionTime() const {return destruction_time;}
    /// Sets this taxon's destruction time
    void SetDestructionTime(double time) {destruction_time = time;}

    /// Add a new organism to this Taxon.
    void AddOrg() { ++num_orgs; ++tot_orgs; }

    /// Add a new offspring Taxon to this one.
    void AddOffspring(Ptr<this_t> offspring_tax) {
      ++num_offspring;
      offspring.insert(offspring_tax);
      AddTotalOffspring();
    }

    /// Recursively increment total offspring count for this and all ancestors
    // Should this be protected or private or something?
    void AddTotalOffspring() {
      ++total_offspring;
      if (parent) { // Keep going until we hit root
        parent->AddTotalOffspring();
      }
    }

    /// Get total number of offspring directly or indirectly
    /// descending from this taxon.
    int GetTotalOffspring(){ return total_offspring; }

    /// Remove an organism from this Taxon (after it dies).
    /// Removals must return true if the taxon needs to continue; false if it should deactivate.
    bool RemoveOrg() {
      emp_assert(num_orgs > 0, num_orgs);
      --num_orgs;

      // If we are out of BOTH organisms and offspring, this Taxon should deactivate.
      return num_orgs;
    }

    /// Remove specified taxon from this taxon's offspring list
    void RemoveFromOffspring(Ptr<this_t> offspring_tax) {
      offspring.erase(offspring_tax);
    }

    /// Remove and offspring taxa after its entire sub-tree has died out (pruning)
    bool RemoveOffspring(Ptr<this_t> offspring_tax) {
      emp_assert(num_offspring > 0, num_offspring, id);
      --num_offspring;
      RemoveFromOffspring(offspring_tax);

      // If we are out of BOTH offspring and organisms, this Taxon should deactivate.
      return num_orgs || num_offspring;
    }

    /// Reduce the total count of extant offspring and recursively do so for
    /// all ancestors (gets called on a taxon's parent when that taxon goes extinct)
    void RemoveTotalOffspring() {
      --total_offspring;
      if (parent) { // Keep going until we hit root
        parent->RemoveTotalOffspring();
      }
    }
  };


  /// A base class for Systematics, maintaining information common to all systematics managers
  /// and providing virtual functions. You probably don't want to instantiate this. It just
  /// exists so that you can make containers of Systematics managers of different types.
  template <typename ORG>
  class SystematicsBase {
  protected:
    bool store_active;        ///< Store all of the currently active taxa?
    bool store_ancestors;     ///< Store all of the direct ancestors from living taxa?
    bool store_outside;       ///< Store taxa that are extinct with no living descendants?
    bool archive;             ///< Set to true if we are supposed to do any archiving of extinct taxa.
    bool store_position;      ///< Keep a vector mapping  positions to pointers
    bool track_synchronous;   ///< Does this systematics manager need to keep track of current and next positions?

    // Stats about active taxa... (totals are across orgs, not taxa)
    size_t org_count;         ///< How many organisms are currently active?
    size_t total_depth;       ///< Sum of taxa depths for calculating average.
    size_t num_roots;         ///< How many distinct injected ancestors are currently in population?
    int max_depth;            ///< Depth of deepest taxon. -1 means needs to be recalculated

    size_t next_id;           ///< What ID value should the next new taxon have?
    size_t curr_update;

    DataManager<double, data::Current, data::Info, data::Range, data::Stats, data::Pull> data_nodes;

  public:
    SystematicsBase(bool _active=true, bool _anc=true, bool _all=false, bool _pos=true)
      : store_active(_active), store_ancestors(_anc), store_outside(_all)
      , archive(store_ancestors || store_outside), store_position(_pos), track_synchronous(false)
      , org_count(0), total_depth(0), num_roots(0), max_depth(0), next_id(0), curr_update(0) { ; }

    virtual ~SystematicsBase(){;}

    using data_node_t = DataNode<double, data::Current, data::Info, data::Range, data::Stats, data::Pull>;
    using data_ptr_t = Ptr<data_node_t>;

    /// Are we tracking a synchronous population?
    bool GetTrackSynchronous() const {return track_synchronous; }

    /// Are we storing all taxa that are still alive in the population?
    bool GetStoreActive() const { return store_active; }

    /// Are we storing all taxa that are the ancestors of living organisms in the population?
    bool GetStoreAncestors() const { return store_ancestors; }

    /// Are we storing all taxa that have died out, as have all of their descendants.
    bool GetStoreOutside() const { return store_outside; }

    /// Are we storing any taxa types that have died out?
    bool GetArchive() const { return archive; }

    /// Are we storing the positions of taxa?
    bool GetStorePosition() const { return store_position; }

    /// How many living organisms are currently being tracked?
    size_t GetTotalOrgs() const { return org_count; }

    /// How many independent trees are being tracked?
    size_t GetNumRoots() const { return num_roots; }

    /// What ID will the next taxon have?
    size_t GetNextID() const {return next_id;}

    /// What is the average phylogenetic depth of organisms in the population?
    double GetAveDepth() const { return ((double) total_depth) / (double) org_count; }

    /// @returns current update/time step
    size_t GetUpdate() const {return curr_update;}

    /// Are we tracking organisms evolving in synchronous generations?
    void SetTrackSynchronous(bool new_val) {track_synchronous = new_val; }

    /// Are we storing all taxa that are still alive in the population?
    void SetStoreActive(bool new_val) { store_active = new_val; }

    /// Are we storing all taxa that are the ancestors of living organisms in the population?
    void SetStoreAncestors(bool new_val) { store_ancestors = new_val; }

    /// Are we storing all taxa that have died out, as have all of their descendants.
    void SetStoreOutside(bool new_val) { store_outside = new_val; }

    /// Are we storing any taxa types that have died out?
    void SetArchive(bool new_val) { archive = new_val; }

    /// Are we storing the location of taxa?
    void SetStorePosition(bool new_val) { store_position = new_val; }

    /// Sets the current update/time step
    void SetUpdate(size_t ud) {curr_update = ud;}

    /// Add a data node to this systematics manager
    /// @param name the name of the data node (so it can be found later)
    data_ptr_t AddDataNode(const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      return &(data_nodes.New(name));
    }

    /// Add a data node to this systematics manager
    /// @param name the name of the data node (so it can be found later)
    /// @param pull_set_fun a function to run when the data node is requested to pull data (returns vector of values)
    data_ptr_t AddDataNode(std::function<emp::vector<double>()> pull_set_fun, const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      auto node = AddDataNode(name);
      node->AddPullSet(pull_set_fun);
      return node;
    }

    /// Add a data node to this systematics manager
    /// @param name the name of the data node (so it can be found later)
    /// @param pull_set_fun a function to run when the data node is requested to pull data (returns single value)
    data_ptr_t AddDataNode(std::function<double()> pull_fun, const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      auto node = AddDataNode(name);
      node->AddPull(pull_fun);
      return node;
    }

    /// @returns a pointer to the data node with the specified name
    data_ptr_t GetDataNode(const std::string & name) {
      return &(data_nodes.Get(name));
    }

    virtual data_ptr_t AddEvolutionaryDistinctivenessDataNode(const std::string & name = "evolutionary_distinctiveness") = 0;
    virtual data_ptr_t AddPairwiseDistanceDataNode(const std::string & name = "pairwise_distance") = 0;
    virtual data_ptr_t AddPhylogeneticDiversityDataNode(const std::string & name = "phylogenetic_diversity") = 0;
    virtual data_ptr_t AddDeleteriousStepDataNode(const std::string & name = "deleterious_steps") = 0;
    virtual data_ptr_t AddVolatilityDataNode(const std::string & name = "volatility") = 0;
    virtual data_ptr_t AddUniqueTaxaDataNode(const std::string & name = "unique_taxa") = 0;
    virtual data_ptr_t AddMutationCountDataNode(const std::string & name = "mutation_count", const std::string & mutation = "substitution") = 0;

    virtual size_t GetNumActive() const = 0;
    virtual size_t GetNumAncestors() const = 0;
    virtual size_t GetNumOutside() const = 0;
    virtual size_t GetTreeSize() const = 0;
    virtual size_t GetNumTaxa() const = 0;
    virtual int GetMaxDepth() = 0;
    virtual int GetPhylogeneticDiversity() const = 0;
    virtual double GetMeanPairwiseDistance(bool branch_only) const = 0;
    virtual double GetSumPairwiseDistance(bool branch_only) const = 0;
    virtual double GetVariancePairwiseDistance(bool branch_only) const = 0;
    virtual emp::vector<double> GetPairwiseDistances(bool branch_only) const = 0;
    virtual int SackinIndex() const = 0;
    virtual double CollessLikeIndex() const = 0;
    virtual std::unordered_map<int, int> GetOutDegreeDistribution() const = 0;
    virtual int GetMRCADepth() const = 0;
    virtual void AddOrg(ORG && org, WorldPosition pos) = 0;
    virtual void AddOrg(ORG & org, WorldPosition pos) = 0;
    virtual void AddOrg(ORG && org, WorldPosition pos, WorldPosition parent) = 0;
    virtual void AddOrg(ORG & org, WorldPosition pos, WorldPosition parent) = 0;
    virtual bool RemoveOrg(WorldPosition pos) = 0;
    virtual void RemoveOrgAfterRepro(WorldPosition pos) = 0;
    virtual void PrintStatus(std::ostream & os) const = 0;
    virtual double CalcDiversity() const = 0;
    virtual void Update() = 0;
    virtual void SetNextParent(WorldPosition pos) = 0;
    virtual void SwapPositions(WorldPosition p1, WorldPosition p2) = 0;
  };

  // Forward-declare CollessStruct for use in calculating Colless metric
  struct CollessStruct;

  /// @brief A tool to track phylogenetic relationships among organisms.
  /// The systematics class tracks the relationships among all organisms based on the INFO_TYPE
  /// provided.  If an offspring has the same value for INFO_TYPE as its parent, it is grouped into
  /// the same taxon.  Otherwise a new Taxon is created and the old one is used as its parent in
  /// the phylogeny.  If the provided INFO_TYPE is the organism's genome, a traditional phylogeny
  /// is formed, with genotypes.  If the organism's behavior/task set is used, then organisms are
  /// grouped by phenotypes.  If the organism's position is used, the evolutionary path through
  /// space is tracked.  Any other aspect of organisms can be tracked this way as well.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT = emp::datastruct::no_data>
  class Systematics : public SystematicsBase<ORG> {
  private:
    using parent_t = SystematicsBase<ORG>;
  public:
    using taxon_t = Taxon<ORG_INFO, DATA_STRUCT>;
    using info_t = ORG_INFO;
  private:
    using hash_t = typename Ptr<taxon_t>::hash_t;
    using fun_calc_info_t = std::function<ORG_INFO(ORG &)>;

    fun_calc_info_t calc_info_fun; ///< Function that takes an organism and returns the unit being tracked by systematics
    Ptr<taxon_t> next_parent;      ///< The taxon that has been marked as parent for next new org
    Ptr<taxon_t> most_recent;      ///< The most-recently added taxon

    using parent_t::store_active;
    using parent_t::store_ancestors;
    using parent_t::store_outside;
    using parent_t::archive;
    using parent_t::store_position;
    using parent_t::track_synchronous;
    using parent_t::org_count;
    using parent_t::total_depth;
    using parent_t::num_roots;
    using parent_t::next_id;
    using parent_t::curr_update;
    using parent_t::max_depth;


  public:
    using typename parent_t::data_ptr_t;
    using parent_t::GetNumActive;
    using parent_t::GetNumAncestors;
    using parent_t::GetNumOutside;
    using parent_t::GetTreeSize;
    using parent_t::GetNumTaxa;
    using parent_t::GetPhylogeneticDiversity;
    using parent_t::GetMeanPairwiseDistance;
    using parent_t::GetSumPairwiseDistance;
    using parent_t::GetVariancePairwiseDistance;
    using parent_t::GetPairwiseDistances;
    using parent_t::GetOutDegreeDistribution;
    using parent_t::GetMRCADepth;
    using parent_t::AddOrg;
    using parent_t::RemoveOrg;
    using parent_t::RemoveOrgAfterRepro;
    using parent_t::PrintStatus;
    using parent_t::CalcDiversity;
    using parent_t::Update;
    using parent_t::GetUpdate;
    using parent_t::SetUpdate;
    using parent_t::SetNextParent;

    using parent_t::GetDataNode;
    using parent_t::AddDataNode;
    using parent_t::AddEvolutionaryDistinctivenessDataNode;
    using parent_t::AddPairwiseDistanceDataNode;
    using parent_t::AddPhylogeneticDiversityDataNode;
    using parent_t::AddDeleteriousStepDataNode;
    using parent_t::AddVolatilityDataNode;
    using parent_t::AddUniqueTaxaDataNode;
    using parent_t::AddMutationCountDataNode;
    using parent_t::GetMaxDepth;

    /// Struct for keeping track of what information to print out in snapshot files
    struct SnapshotInfo {
      using snapshot_fun_t = std::function<std::string(const taxon_t &)>;
      snapshot_fun_t fun; ///< Function for converting taxon to string containing desired data
      std::string key;    ///< Column name for data calculated with this function
      std::string desc;   ///< Description of data in this function

      SnapshotInfo(const snapshot_fun_t & _fun, const std::string & _key, const std::string & _desc="")
        : fun(_fun),
          key(_key),
          desc(_desc)
      { ; }
    };

    emp::vector<SnapshotInfo> user_snapshot_funs; ///< Collection of all desired snapshot file columns

    std::unordered_set< Ptr<taxon_t>, hash_t > active_taxa;   ///< A set of all living taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > ancestor_taxa; ///< A set of all dead, ancestral taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > outside_taxa;  ///< A set of all dead taxa w/o descendants.

    Ptr<taxon_t> to_be_removed = nullptr; ///< Taxon to remove org from after next call to AddOrg
    emp::WorldPosition removal_pos = {0, 0};   ///< Position of taxon to next be removed

    emp::vector<emp::vector<Ptr<taxon_t> > > taxon_locations; ///< Positions in this vector indicate taxon positions in world

    Signal<void(Ptr<taxon_t>, ORG & org)> on_new_sig; ///< Trigger when a new taxon is created
    Signal<void(Ptr<taxon_t>)> on_extinct_sig; ///< Trigger when a taxon goes extinct
    Signal<void(Ptr<taxon_t>)> on_prune_sig; ///< Trigger when any organism is pruned from tree

    mutable Ptr<taxon_t> mrca;  ///< Most recent common ancestor in the population.

    /// Called whenever a taxon has no organisms AND no descendants.
    void Prune(Ptr<taxon_t> taxon);

    /// Called when an offspring taxa has been deleted.
    void RemoveOffspring(Ptr<taxon_t> offspring, Ptr<taxon_t> taxon);

    /// Called when there are no more living members of a taxon.  There may be descendants.
    void MarkExtinct(Ptr<taxon_t> taxon);

    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    /// Helper function for RemoveBefore
    /// @returns true if a a taxon can safely be
    /// removed by RemoveBefore
    bool CanRemove(Ptr<taxon_t> t, int ud);
    // Helper for Colless function calculation
    CollessStruct RecursiveCollessStep(Ptr<taxon_t> curr) const;
    #endif // DOXYGEN_SHOULD_SKIP_THIS



  public:

    /**
     * Contructor for Systematics; controls what information should be stored.
     * @param store_active     Should living organisms' taxa be tracked? (typically yes!)
     * @param store_ancestors  Should ancestral organisms' taxa be maintained?  (yes for lineages!)
     * @param store_all        Should all dead taxa be maintained? (typically no; it gets BIG!)
     * @param store_pos        Should the systematics tracker keep track of organism positions?
     *                         (probably yes - only turn this off if you know what you're doing)
     */

    Systematics(fun_calc_info_t calc_taxon, bool store_active=true, bool store_ancestors=true, bool store_all=false, bool store_pos=true)
      : parent_t(store_active, store_ancestors, store_all, store_pos)
      , calc_info_fun(calc_taxon)
      , active_taxa(), ancestor_taxa(), outside_taxa()
      , mrca(nullptr) { ; }
    Systematics(const Systematics &) = delete;
    Systematics(Systematics &&) = default;
    ~Systematics() {
      for (auto x : active_taxa) x.Delete();
      for (auto x : ancestor_taxa) x.Delete();
      for (auto x : outside_taxa) x.Delete();
      active_taxa.clear();
      ancestor_taxa.clear();
      outside_taxa.clear();
    }

    // ===== Functions for modifying phylogeny/systematics manager internal state ====

    /// Switch to next update/time step
    /// Useful for keeping track of taxon survival times
    /// and population positions in synchronous generation worlds.
    void Update();

    ///@{
    /// Add information about a new organism, including its stored info and parent's taxon;
    /// If you would like the systematics manager to track taxon age, you can also supply
    /// the update at which the taxon is being added.
    /// return a pointer for the associated taxon.
    /// @returns a pointer for the associated taxon.
    /// @param org a reference to the organism being added
    /// @param pos the position of the organism being added
    /// @param parent a pointer to the org's parent
    void AddOrg(ORG && org, WorldPosition pos);
    void AddOrg(ORG && org, WorldPosition pos, WorldPosition parent);
    Ptr<taxon_t> AddOrg(ORG && org, WorldPosition pos, Ptr<taxon_t> parent);
    Ptr<taxon_t> AddOrg(ORG && org, Ptr<taxon_t> parent=nullptr);

    void AddOrg(ORG & org, WorldPosition pos);
    void AddOrg(ORG & org, WorldPosition pos, WorldPosition parent);
    Ptr<taxon_t> AddOrg(ORG & org, WorldPosition pos, Ptr<taxon_t> parent);
    Ptr<taxon_t> AddOrg(ORG & org, Ptr<taxon_t> parent=nullptr);
    ///@}

    ///@{
    /// Remove an instance of an organism; track when it's gone.
    /// @param pos the world position of the individual being removed
    /// @param taxon a pointer to the taxon of the individual being removed
    bool RemoveOrg(WorldPosition pos);
    bool RemoveOrg(Ptr<taxon_t> taxon);
    ///@}

    ///@{
    /// Mark an instance of a taxon to be removed; track when it's gone.
    /// This is a work-around to deal with steady state/non-synchronous
    /// populations in which an organism might die as its offspring is born
    /// (e.g. in a spatial world where the offspring replaces the parent).
    /// If the bookkeeping is not handled correctly, we could accidentally
    /// mark the taxon as extinct when it is actually continuing.
    /// By using this method, the taxon won't be removed until after the
    /// next org is added or the next time an org is marked for removal.
    /// @param pos the world position of the individual being removed
    /// @param taxon a pointer to the taxon of the individual being removed
    void RemoveOrgAfterRepro(WorldPosition pos);
    void RemoveOrgAfterRepro(Ptr<taxon_t> taxon);
    ///@}


    ///@{
    /// Tell systematics manager that the parent of the next taxon added
    /// will be the one specified by this function (either at the specified
    /// position or the one pointed to by the given pointer)
    /// Works with version of AddOrg that only takes org, position, and
    /// update.
    /// Will be set to null after being assigned as the parent of a taxon
    void SetNextParent(WorldPosition pos) {
      emp_assert(pos.IsActive() || !pos.IsValid());
      if (!pos.IsValid()) {
        next_parent = nullptr;
      } else {
        next_parent = taxon_locations[pos.GetPopID()][pos.GetIndex()];
      }
    }

    void SetNextParent(Ptr<taxon_t> p) {
      next_parent = p;
    }
    ///@}

    /// Set function used to calculate taxons from organisms
    void SetCalcInfoFun(fun_calc_info_t f) {calc_info_fun = f;}

    /// Remove all taxa that 1) went extinct before the specified update/time step,
    /// and 2) only have ancestors that went extinct before the specified update/time step.
    /// Warning: this function invalidates most measurements you could make about tree topology.
    /// It is useful in select situations where you need to store ancestors for some period of time,
    /// but cannot computationally afford to store all ancestors for your entire run.
    void RemoveBefore(int ud);

    /// Apply @param fun to every active taxon (const version)
    void ApplyToActiveTaxa(const std::function<void(const emp::Ptr<taxon_t> tax)> & fun) const {
      std::for_each(active_taxa.begin(), active_taxa.end(), fun);
    }

    /// Apply @param fun to every active taxon
    void ApplyToActiveTaxa(const std::function<void(emp::Ptr<taxon_t> tax)> & fun) {
      std::for_each(active_taxa.begin(), active_taxa.end(), fun);
    }

    /// Apply @param fun to every ancestor taxon (const version)
    void ApplyToAncestorTaxa(const std::function<void(const emp::Ptr<taxon_t> tax)> & fun) const {
      std::for_each(ancestor_taxa.begin(), ancestor_taxa.end(), fun);
    }

    /// Apply @param fun to every ancestor taxon
    void ApplyToAncestorTaxa(const std::function<void(emp::Ptr<taxon_t> tax)> & fun) {
      std::for_each(ancestor_taxa.begin(), ancestor_taxa.end(), fun);
    }

    /// Apply @param fun to every outside taxon (const version)
    void ApplyToOutsideTaxa(const std::function<void(const emp::Ptr<taxon_t> tax)> & fun) const {
      std::for_each(outside_taxa.begin(), outside_taxa.end(), fun);
    }

    /// Apply @param fun to every outside taxon
    void ApplyToOutsideTaxa(const std::function<void(emp::Ptr<taxon_t> tax)> & fun) {
      std::for_each(outside_taxa.begin(), outside_taxa.end(), fun);
    }

    /// Run given function on all taxa (const version)
    void ApplyToAllTaxa(const std::function<void(const emp::Ptr<taxon_t> tax)> & fun) const {
      ApplyToActiveTaxa(fun);
      ApplyToAncestorTaxa(fun);
      ApplyToOutsideTaxa(fun);
    }

    /// Run given function on all taxa
    void ApplyToAllTaxa(const std::function<void(emp::Ptr<taxon_t> tax)> & fun) {
      ApplyToActiveTaxa(fun);
      ApplyToAncestorTaxa(fun);
      ApplyToOutsideTaxa(fun);
    }

    /// Run given function on all taxa and return result (const version)
    template <typename T>
    emp::vector<T> ApplyToAllTaxa(const std::function<T(const emp::Ptr<taxon_t> tax)> & fun) const {
      emp::vector<T> result;
      // const auto all = {std::ranges::ref_view(active_taxa), std::ranges::ref_view(ancestor_taxa),
      //             std::ranges::ref_view(outside_taxa)};
      // for (emp::Ptr<taxon_t> tax : all | std::views::join) {
      //   result.push_back(fun(tax));
      // }
      for (emp::Ptr<taxon_t> tax : active_taxa) {
        result.push_back(fun(tax));
      }
      for (emp::Ptr<taxon_t> tax : ancestor_taxa) {
        result.push_back(fun(tax));
      }
      for (emp::Ptr<taxon_t> tax : ancestor_taxa) {
        result.push_back(fun(tax));
      }
      return result;
    }

    /// Run given function on all taxa and return result
    template <typename T>
    emp::vector<T> ApplyToAllTaxa(const std::function<T(emp::Ptr<taxon_t> tax)> & fun) {
      emp::vector<T> result;
      // const auto all = {std::ranges::ref_view(active_taxa),
      //             std::ranges::ref_view(ancestor_taxa),
      //             std::ranges::ref_view(outside_taxa)};
      // for (emp::Ptr<taxon_t> tax : all | std::views::join) {
      for (emp::Ptr<taxon_t> tax : active_taxa) {
        result.push_back(fun(tax));
      }
      for (emp::Ptr<taxon_t> tax : ancestor_taxa) {
        result.push_back(fun(tax));
      }
      for (emp::Ptr<taxon_t> tax : ancestor_taxa) {
        result.push_back(fun(tax));
      }

      return result;
    }

    // ===== Functions for querying phylogeny/systematics manager internal state ====

    // Currently using raw pointer because of a weird bug in emp::Ptr. Should switch when fixed.
    std::unordered_set< Ptr<taxon_t>, hash_t > * GetActivePtr() { return &active_taxa; }
    /// @returns set of active (extant/living) taxa0
    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetActive() const { return active_taxa; }
    /// @returns set of ancestor taxa (extinct, but have active descendants)
    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetAncestors() const { return ancestor_taxa; }
    /// @returns set of outside taxa (extinct, with no active descendants)
    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetOutside() const { return outside_taxa; }

    /// How many taxa are still active in the population?
    size_t GetNumActive() const { return active_taxa.size(); }

    /// How many taxa are ancestors of living organisms (but have died out themselves)?
    size_t GetNumAncestors() const { return ancestor_taxa.size(); }

    /// How many taxa are stored that have died out, as have their descendents?
    size_t GetNumOutside() const { return outside_taxa.size(); }

    /// How many taxa are in the current phylogeny?
    size_t GetTreeSize() const { return GetNumActive() + GetNumAncestors(); }

    /// How many taxa are stored in total?
    size_t GetNumTaxa() const { return GetTreeSize() + GetNumOutside(); }

    /// @returns the phylogenetic depth (lineage length) of the taxon with
    /// the longest lineage out of all active taxa
    int GetMaxDepth();

    /// @returns the taxon that will be used as the parent
    /// of the next taxon created via the version of AddOrg
    /// that does not accept a parent
    Ptr<taxon_t> GetNextParent() {
      return next_parent;
    }

    /// @returns the most recently created taxon
    Ptr<taxon_t> GetMostRecent() {
      return most_recent;
    }

    /// @returns a pointer to the parent of a given taxon
    Ptr<taxon_t> Parent(Ptr<taxon_t> taxon) const;

    /// @returns true if there is a taxon at specified location
    bool IsTaxonAt(WorldPosition id) {
      emp_assert(id.GetPopID() < taxon_locations.size(), "Invalid population id", id, taxon_locations.size());
      emp_assert(id.GetIndex() < taxon_locations[id.GetPopID()].size(), "Invalid taxon location", id, taxon_locations[id.GetPopID()].size());
      return taxon_locations[id.GetPopID()][id.GetIndex()] != nullptr;
    }

    /// @returns pointer to taxon at specified location
    Ptr<taxon_t> GetTaxonAt(WorldPosition id) {
      emp_assert(id.GetPopID() < taxon_locations.size(), "Invalid population id", id, taxon_locations.size());
      emp_assert(id.GetIndex() < taxon_locations[id.GetPopID()].size(), "Invalid taxon location", id, taxon_locations[id.GetPopID()].size());
      return taxon_locations[id.GetPopID()][id.GetIndex()];
    }

    // ===== Functions for adding actions to systematics manager signals ====

    /// Provide a function for Systematics to call each time a new taxon is created.
    /// Trigger:  New taxon is made
    /// Argument: Pointer to taxon, reference to org taxon was created from
    SignalKey OnNew(std::function<void(Ptr<taxon_t> t, ORG & org)> & fun) { return on_new_sig.AddAction(fun); }

    /// Provide a function for Systematics to call each time a taxon goes extinct.
    /// Trigger:  Taxon is going extinct
    /// Argument: Pointer to taxon
    SignalKey OnExtinct(std::function<void(Ptr<taxon_t> t)> & fun) { return on_extinct_sig.AddAction(fun); }

    /// Provide a function for Systematics to call each time a taxon is about to be pruned (removed from ancestors).
    /// Trigger:  Taxon is about to be killed
    /// Argument: Pointer to taxon
    SignalKey OnPrune(std::function<void(Ptr<taxon_t>)> & fun) { return on_prune_sig.AddAction(fun); }

    // ===== Functions for adding data nodes to systematics manager ====

    /// Add data node that records evolutionary distinctiveness when requested to pull.
    /// Used by AddPhylodiversityFile in World_output.hpp
    virtual data_ptr_t
    AddEvolutionaryDistinctivenessDataNode(const std::string & name = "evolutionary_distinctiveness") {
      auto node = AddDataNode(name);
      node->AddPullSet([this](){
        emp::vector<double> result;
        for (auto tax : active_taxa) {
          result.push_back(GetEvolutionaryDistinctiveness(tax, curr_update));
        }
        return result;
      });

      return node;
    }

    /// Add data node that records pairwise distance when requested to pull.
    /// Used by AddPhylodiversityFile in World_output.hpp
    virtual data_ptr_t AddPairwiseDistanceDataNode(const std::string & name = "pairwise_distance") {
      auto node = AddDataNode(name);
      node->AddPullSet([this](){
        return GetPairwiseDistances();
      });
      return node;
    }

    /// Add data node that records phylogenetic distinctiveness when requested to pull.
    /// Used by AddPhylodiversityFile in World_output.hpp
    virtual data_ptr_t AddPhylogeneticDiversityDataNode(const std::string & name = "phylogenetic_diversity") {
      auto node = AddDataNode(name);
      node->AddPull([this](){
        return GetPhylogeneticDiversity();
      });
      return node;
    }

    /// Add data node that records counts of deleterious steps along
    /// lineages in this systematics manager when requested to pull.
    /// Used by AddLineageMutationFile in World_output.hpp
    virtual data_ptr_t
    AddDeleteriousStepDataNode(const std::string & name = "deleterious_steps") {
      auto node = AddDataNode(name);

      if constexpr (!DATA_STRUCT::has_fitness_t::value) {
        emp_assert(false &&
          "Error: Trying to track deleterious steps in Systematics manager that doesn't track fitness" &&
          "Please use a DATA_STRUCT type that supports fitness tracking.");
      } else {
        node->AddPullSet([this](){
          emp::vector<double> result;
          for (auto tax : active_taxa) {
            result.push_back(CountDeleteriousSteps(tax));
          }
          return result;
        });
      }
      return node;
    }

    /// Add data node that phenotypic volatility (changes in phenotype) along
    /// lineages in this systematics manager when requested to pull.
    /// Used by AddLineageMutationFile in World_output.hpp
    virtual data_ptr_t
    AddVolatilityDataNode(const std::string & name = "volatility") {
      auto node = AddDataNode(name);

      if constexpr (!DATA_STRUCT::has_phen_t::value) {
        emp_assert(false &&
          "Error: Trying to track phenotypic volatility in Systematics manager that doesn't track fitness" &&
          "Please use a DATA_STRUCT type that supports phenotype tracking.");
      } else {
        node->AddPullSet([this](){
          emp::vector<double> result;
          for (auto tax : active_taxa) {
            result.push_back(CountPhenotypeChanges(tax));
          }
          return result;
        });
      }
      return node;
    }

    /// Add data node that records counts of unique taxa along
    /// lineages in this systematics manager when requested to pull.
    /// Used by AddLineageMutationFile in World_output.hpp
    virtual data_ptr_t
    AddUniqueTaxaDataNode(const std::string & name = "unique_taxa") {
      auto node = AddDataNode(name);

      if constexpr (!DATA_STRUCT::has_phen_t::value) {
        emp_assert(false &&
          "Error: Trying to track phenotypic volatility in Systematics manager that doesn't track fitness" &&
          "Please use a DATA_STRUCT type that supports phenotype tracking.");
      } else {

        node->AddPullSet([this](){
          emp::vector<double> result;
          for (auto tax : active_taxa) {
            result.push_back(CountUniquePhenotypes(tax));
          }
          return result;
        });
      }

      return node;
    }

    /// Add data node that records counts of mutations of the specified type along
    /// lineages in this systematics manager when requested to pull.
    /// Used by AddLineageMutationFile in World_output.hpp
    virtual data_ptr_t
    AddMutationCountDataNode(const std::string & name = "mutation_count", const std::string & mutation = "substitution") {
      auto node = AddDataNode(name);

      if constexpr (!DATA_STRUCT::has_mutations_t::value) {
        emp_assert(false &&
          "Error: Trying to track phenotypic volatility in Systematics manager that doesn't track mutations" &&
          "Please use a DATA_STRUCT type that supports mutation tracking.");
      } else {
        node->AddPullSet([this,mutation](){
          emp::vector<double> result;
          for (auto tax : active_taxa) {
            result.push_back(CountMuts(tax, mutation));
          }
          return result;
        });
      }
      return node;
    }


    // ===== Functions for calculating phylogeny topology metrics ====

    /** From (Faith 1992, reviewed in Winters et al., 2013), phylogenetic diversity is
     *  the sum of edges in the minimal spanning tree connected the taxa you're
     *  calculating diversity of.
     *
     * This calculates phylogenetic diversity for all extant taxa in the tree, assuming
     * all edges from parent to child have a length of one. Possible extensions to this
     * function that might be useful in the future include:
     * - Pass it a set of taxon_t pointers and have it calculate PD for just those taxa
     * - Enable calculation of branch lengths by amount of time that elapsed between
     *   origination of parent and origination of offspring
     * - Enable a paleontology compatibility mode where only branching points are calculated
     */
    int GetPhylogeneticDiversity() const {
      // As shown on page 5 of Faith 1992, when all branch lengths are equal the phylogenetic
      // diversity is the number of internal nodes plus the number of extant taxa - 1.
      //int phylodiversity = ancestor_taxa.size() + active_taxa.size() -1;

      return ancestor_taxa.size() + active_taxa.size() - 1;
    }


    /// @returns phylogenetic diversity if used without any arguments .
    /// If you want to receive normalized data, you need to include the number of generations
    /// your tree has (multiples of 10 from 10 to 100 are allowed)
    /// you also need to specify a file with which to normalize your data.
    /// If value is outside of the values in the file, 100th percentile will be returned
    int GetPhylogeneticDiversityNormalize(int generation = 0, std::string filename = "") const;


    /** This is a metric of how distinct \c tax is from the rest of the population.
     *
     * (From Vane-Wright et al., 1991; reviewed in Winter et al., 2013) */
    double GetTaxonDistinctiveness(Ptr<taxon_t> tax) const {return 1.0/GetDistanceToRoot(tax);}

    /** This metric (from Isaac, 2007; reviewed in Winter et al., 2013) measures how
     * distinct \c tax is from the rest of the population, weighted for the amount of
     * unique evolutionary history that it represents.
     *
     * To quantify length of evolutionary history, this method needs \c time: the current
     * time, in whatever units time is being measured in when taxa are added to the systematics
     * manager. Note that passing a time in the past will produce inaccurate results (since we
     * don't know what the state of the tree was at that time).
     *
     * Assumes the tree is all connected. Will return -1 if this assumption isn't met.*/
    double GetEvolutionaryDistinctiveness(Ptr<taxon_t> tax, double time) const;

    /** Calculates mean pairwise distance between extant taxa (Webb and Losos, 2000).
     * This measurement is also called Average Taxonomic Diversity (Warwick and Clark, 1998)
     * (for demonstration of equivalence see Tucker et al, 2016). This measurement tells
     * you about the amount of distinctness in the community as a whole.
     *
     * This measurement assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred). */
    double GetMeanPairwiseDistance(bool branch_only=false) const {
      emp::vector<double> dists = GetPairwiseDistances(branch_only);
      return (double)Sum(dists)/dists.size();
    }

    /** Calculates summed pairwise distance between extant taxa. Tucker et al 2017 points
     *  out that this is a measure of phylogenetic richness.
     *
     * This measurement assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred) */
    double GetSumPairwiseDistance(bool branch_only=false) const {
      emp::vector<double> v = GetPairwiseDistances(branch_only);
      return Sum(v);
    }

    /** Calculates variance of pairwise distance between extant taxa. Tucker et al 2017 points
     *  out that this is a measure of phylogenetic regularity.
     *
     * This measurement assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred). */
    double GetVariancePairwiseDistance(bool branch_only=false) const {
      emp::vector<double> v = GetPairwiseDistances(branch_only);
      return Variance(v);
    }
    /** Calculates a vector of all pairwise distances between extant taxa.
     *
     * This method assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred). * */
    emp::vector<double> GetPairwiseDistances(bool branch_only=false) const;


    /**
     * Returns a vector containing all taxa that were extant at \c time_point and
     * were at that time the most recent ancestors of taxa that are now extant
     * Example: Say the only current extant taxon is C, its lineage goes A -> B -> C,
     * and B and C were both alive at the specified time_point. This function would
     * only return B. If, however, there were another currently extant taxon that were
     * descended directly from A, then this function would return both A and B. */
    std::set<Ptr<taxon_t>> GetCanopyExtantRoots(int time_point = 0) const;


    /** Counts the total number of ancestors between @param tax and MRCA, if there is one. If
     *  there is no common ancestor, distance to the root of this tree is calculated instead.*/
    int GetDistanceToRoot(Ptr<taxon_t> tax) const ;

    /** Counts the number of branching points leading to multiple extant taxa
     * between @param tax and the most-recent common ancestor (or the root of its subtree,
     * if no MRCA exists). This is useful because a lot
     * of stats for phylogenies are designed for phylogenies reconstructed from extant taxa.
     * These phylogenies generally only contain branching points, rather than every ancestor
     * along the way to the current taxon.*/
    int GetBranchesToRoot(Ptr<taxon_t> tax) const;

    /** Calculate Sackin Index of this tree (Sackin, 1972; reviewed in Shao, 1990).
     * Measures tree balance*/
    int SackinIndex() const {
      int sackin = 0;
      for (auto taxon : active_taxa) {
        sackin += GetBranchesToRoot(taxon) + 1; // Sackin index counts root as branch
      }
      return sackin;
    }

    /** Returns dictionary containing a histogram of node out degrees
     * e.g. {1:4, 2:10, 3:4} means the tree has 4 unifurcations,
     * 10 bifurcations, and 4 trifurcations
     * **/
    std::unordered_map<int, int> GetOutDegreeDistribution() const {
      std::unordered_map<int, int> dist;
      ApplyToAllTaxa([&dist](emp::Ptr<taxon_t> tax){emp::IncrementCounter(dist, tax->GetNumOff());});
      return dist;
    }

    /** Get average origin time for whole phylogeny.
     * If @param normalize is set to true, will apply normalization to make result
     * comparable to what you would expect from a strictly bifurcating tree (as most
     * reconstruction methods will produce). This normalization is achieved by multiplying
     * each taxon's values by the number of offspring taxa it has minus one.
    */
    double GetAverageOriginTime(bool normalize=false) {
      double total = 0;
      double count = 0;
      // const auto all = {std::ranges::ref_view(active_taxa),
      //                   std::ranges::ref_view(ancestor_taxa),
      //                   std::ranges::ref_view(outside_taxa)};
      // for (emp::Ptr<taxon_t> tax : all | std::views::join) {
      for (emp::Ptr<taxon_t> tax : active_taxa) {
        double weight = 1;
        if (normalize) {
          weight = std::max(0, (int)tax->GetNumOff() - 1);
        }
        total += tax->GetOriginationTime() * weight;
        count += weight;
      }
      for (emp::Ptr<taxon_t> tax : ancestor_taxa) {
        double weight = 1;
        if (normalize) {
          weight = std::max(0, (int)tax->GetNumOff() - 1);
        }
        total += tax->GetOriginationTime() * weight;
        count += weight;
      }
      for (emp::Ptr<taxon_t> tax : outside_taxa) {
        double weight = 1;
        if (normalize) {
          weight = std::max(0, (int)tax->GetNumOff() - 1);
        }
        total += tax->GetOriginationTime() * weight;
        count += weight;
      }

      if (count == 0) {
        return 0;
      }
      return total/count;
    }

    /** Calculate Colless Index of this tree (Colless, 1982; reviewed in Shao, 1990).
     * Measures tree balance. The standard Colless index only works for bifurcating trees,
     * so this will be a Colless-like Index, as suggested in
     * "Sound Colless-like balance indices for multifurcating trees" (Mir, 2018, PLoS One)*/
    double CollessLikeIndex() const {
      GetMRCA();
      return RecursiveCollessStep(mrca).total;
    }

    /// @returns a pointer to the Most-Recent Common Ancestor for the population.
    Ptr<taxon_t> GetMRCA() const;

    /// @returns the depth of the Most-Recent Common Ancestor; return -1 for none.
    int GetMRCADepth() const;

    /// @returns a pointer to the Most-Recent Ancestor shared by two taxa.
    Ptr<taxon_t> GetSharedAncestor(Ptr<taxon_t> t1, Ptr<taxon_t> t2) const;

    /// @returns the genetic diversity of the population.
    double CalcDiversity() const;

    /// @returns vector containing the lineages of the specified taxon
    emp::vector<Ptr<taxon_t>> GetLineage(Ptr<taxon_t> tax) const {
      emp::vector<Ptr<taxon_t>> lineage;
      lineage.push_back(tax);

      while (tax) {
        tax = Parent(tax);
        lineage.push_back(tax);
      }
      return lineage;
    }

    /// @returns vector containing the lineages of the specified taxon
    /// up to and including the MRCA, but not past the MRCA
    emp::vector<Ptr<taxon_t>> GetLineageToMRCA(Ptr<taxon_t> tax) const {
      GetMRCA();
      emp::vector<Ptr<taxon_t>> lineage;
      lineage.push_back(tax);

      while (tax && tax != mrca) {
        tax = Parent(tax);
        lineage.push_back(tax);
      }
      return lineage;
    }


    // ===== Output functions ====

    /// Print details about the Systematics manager.
    /// First prints setting, followed by all active, ancestor, and outside
    /// taxa being stored. Format for taxa is
    /// [ id | number of orgs in this taxon, number of offspring taxa of this taxon | parent taxon]
    /// @param os output stream to print to
    void PrintStatus(std::ostream & os=std::cout) const;

    /// Print a whole lineage. Format: "Lineage:", followed by each taxon in the lineage, each on new line
    /// @param taxon a pointer to the taxon to print the lineage of
    /// @param os output stream to print to
    void PrintLineage(Ptr<taxon_t> taxon, std::ostream & os=std::cout) const;

    /// Add a new snapshot function.
    /// When a snapshot of the systematics is taken, in addition to the default
    /// set of functions, all user-added snapshot functions are run. Functions
    /// take a reference to a taxon as input and return the string to be dumped
    /// in the file at the given key.
    void AddSnapshotFun(const std::function<std::string(const taxon_t &)> & fun,
                        const std::string & key, const std::string & desc="") {
      user_snapshot_funs.emplace_back(fun, key, desc);
    }

    /// Take a snapshot of current state of taxon phylogeny.
    /// WARNING: Current, this function assumes one parent taxon per-taxon.
    /// @param file_path the file to store the snapshot data in
    void Snapshot(const std::string & file_path) const;

    void SwapPositions(WorldPosition p1, WorldPosition p2) {
      emp::vector<Ptr<taxon_t> > & v1 = taxon_locations[p1.GetPopID()];
      emp::vector<Ptr<taxon_t> > & v2 = taxon_locations[p2.GetPopID()];
      std::swap(v1[p1.GetIndex()], v2[p2.GetIndex()]);
    }

    void LoadFromFile(const std::string & file_path, const std::string & info_col = "info",
                                                             bool assume_leaves_extant=true);

  };

  // =============================================================
  // ===                                                       ===
  // ===  Out-of-class member function definitions from above  ===
  // ===                                                       ===
  // =============================================================

  // ======= Functions for manipulating systematics manager internals

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::Update() {
    if (track_synchronous) {

      // Clear pending removal
      if (to_be_removed != nullptr) {
        RemoveOrg(to_be_removed);
        taxon_locations[removal_pos.GetPopID()][removal_pos.GetIndex()] = nullptr;
        to_be_removed = nullptr;
        removal_pos = {0, 0};
      }

      // Assumes that synchronous worlds have two populations, with 0
      // being currently alive and 1 being the one being created
      std::swap(taxon_locations[0], taxon_locations[1]);
      taxon_locations[1].resize(0);
    }
    ++curr_update;
  }

  // Should be called wheneven a taxon has no organisms AND no descendants.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::Prune(Ptr<taxon_t> taxon) {
    on_prune_sig.Trigger(taxon);
    RemoveOffspring( taxon, taxon->GetParent() );           // Notify parent of the pruning.
    if (store_ancestors) ancestor_taxa.erase(taxon); // Clear from ancestors set (if there)
    if (store_outside) outside_taxa.insert(taxon);   // Add to outside set (if tracked)
    else {
      if (taxon == mrca) {
        mrca = nullptr;
      }
      taxon.Delete();                             //  ...or else get rid of it.
    }
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOffspring(Ptr<taxon_t> offspring, Ptr<taxon_t> taxon) {
    if (!taxon) { num_roots--; return; }             // Offspring was root; remove and return.
    bool still_active = taxon->RemoveOffspring(offspring);    // Taxon still active w/ 1 fewer offspring?
    if (!still_active) Prune(taxon);                 // If out of offspring, remove from tree.

    // If the taxon is still active AND the is the current mrca AND now has only one offspring,
    // clear the MRCA for lazy re-evaluation later.
    else if (taxon == mrca && taxon->GetNumOff() == 1) {
      mrca = nullptr;
    }
  }

  // Mark a taxon extinct if there are no more living members.  There may be descendants.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::MarkExtinct(Ptr<taxon_t> taxon) {
    emp_assert(taxon);
    emp_assert(taxon->GetNumOrgs() == 0);

    // Track destruction time
    taxon->SetDestructionTime(curr_update);

    // Give other functions a chance to do stuff with taxon before extinction
    on_extinct_sig.Trigger(taxon);

    if (max_depth == (int)taxon->GetDepth()) {
      // We no longer know the max depth
      max_depth = -1;
    }

    if (taxon->GetParent()) {
      // Update extant descendant count for all ancestors
      taxon->GetParent()->RemoveTotalOffspring();
    }

    if (store_active) active_taxa.erase(taxon);
    if (!archive) {   // If we don't archive taxa, delete them.
      for (Ptr<taxon_t> off_tax : taxon->GetOffspring()) {
        off_tax->NullifyParent();
      }

      taxon.Delete();
      return;
    }

    if (store_ancestors) {
      ancestor_taxa.insert(taxon);  // Move taxon to ancestors...
    }
    if (taxon == mrca && taxon->GetNumOff() <= 1) {
      // If this taxon was mrca and has only one offspring, then the new
      // mrca is somewhere farther down the chain.
      // If this taxon was mrca and now has no offspring, something very
      // strange has happened.
      // Either way, we should mark mrca for lazy recalculation
      mrca = nullptr;
    }
    if (taxon->GetNumOff() == 0) Prune(taxon);         // ...and prune from there if needed.
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // Can't return a pointer for the associated taxon because of obnoxious inheritance problems
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  // Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, WorldPosition pos) {
    emp_assert(store_position, "Trying to pass position to a systematics manager that can't use it");
    // emp_assert(next_parent, "Adding organism with no parent specified and no next_parent set");
    AddOrg(org, pos, next_parent);
    next_parent = nullptr;
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // Can't return a pointer for the associated taxon because of obnoxious inheritance problems
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  // Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, WorldPosition pos) {
    emp_assert(store_position, "Trying to pass position to a systematics manager that can't use it");
    // emp_assert(next_parent, "Adding organism with no parent specified and no next_parent set");
    AddOrg(org, pos, next_parent);
    next_parent = nullptr;
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // Can't return a pointer for the associated taxon because of obnoxious inheritance problems
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  // Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, WorldPosition pos, WorldPosition parent) {
    emp_assert(store_position, "Trying to pass position to a systematics manager that can't use it");
    AddOrg(org, pos, taxon_locations[parent.GetPopID()][parent.GetIndex()]);
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // Can't return a pointer for the associated taxon because of obnoxious inheritance problems
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  // Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, WorldPosition pos, WorldPosition parent) {
    emp_assert(store_position, "Trying to pass position to a systematics manager that can't use it");
    AddOrg(org, pos, taxon_locations[parent.GetPopID()][parent.GetIndex()]);
  }


  // Version for if you aren't tracking positions
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, Ptr<taxon_t> parent) {
    emp_assert(!store_position &&
              "Trying to add org to position-tracking systematics manager without position. Either specify a valid position or turn of position tracking for systematic manager.", store_position);
    return AddOrg(org, WorldPosition::invalid_id, parent);
  }

  // Version for if you aren't tracking positions
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, Ptr<taxon_t> parent) {
    emp_assert(!store_position &&
              "Trying to add org to position-tracking systematics manager without position. Either specify a valid position or turn of position tracking for systematic manager.", store_position);
    return AddOrg(org, WorldPosition::invalid_id, parent);
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // return a pointer for the associated taxon.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, WorldPosition pos, Ptr<taxon_t> parent) {
    return AddOrg(org, pos, parent);
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, WorldPosition pos, Ptr<taxon_t> parent) {
    org_count++;                  // Keep count of how many organisms are being tracked.

    ORG_INFO info = calc_info_fun(org);

    Ptr<taxon_t> cur_taxon = parent;

    // If this organism needs a new taxon, build it!
    if (!cur_taxon || cur_taxon->GetInfo() != info) {
      if (!cur_taxon) {                                 // No parent -> NEW tree
        num_roots++;                                    // ...track extra root.
        mrca = nullptr;                                 // ...nix old common ancestor
      }

      cur_taxon = NewPtr<taxon_t>(++next_id, info, parent);  // Build new taxon.
      if (max_depth != -1 && (int)cur_taxon->GetDepth() > max_depth) {
        max_depth = cur_taxon->GetDepth();
      }

      if (store_active) active_taxa.insert(cur_taxon);       // Store new taxon.
      if (parent) parent->AddOffspring(cur_taxon);           // Track tree info.

      cur_taxon->SetOriginationTime(curr_update);
      on_new_sig.Trigger(cur_taxon, org);
    }
    // std::cout << "about to store poisition" << std::endl;
    if (store_position) {
      if (pos.GetPopID() >= taxon_locations.size()) {
        taxon_locations.resize(pos.GetPopID()+1);
      }
      if (pos.GetIndex() >= taxon_locations[pos.GetPopID()].size()) {
        taxon_locations[pos.GetPopID()].resize(pos.GetIndex()+1);
      }
      taxon_locations[pos.GetPopID()][pos.GetIndex()] = cur_taxon;
    }

    cur_taxon->AddOrg();                    // Record the current organism in its taxon.
    total_depth += cur_taxon->GetDepth();   // Track the total depth (for averaging)

    if (to_be_removed) {
      RemoveOrg(to_be_removed);
      to_be_removed = nullptr;
    }

    most_recent = cur_taxon;
    return cur_taxon;                       // Return the taxon used.
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrgAfterRepro(WorldPosition pos) {
    emp_assert(store_position, "Trying to remove org based on position from systematics manager that doesn't track it.");

    if (pos.GetPopID() >= taxon_locations.size() ||
        pos.GetIndex() >= taxon_locations[pos.GetPopID()].size() ||
        !taxon_locations[pos.GetPopID()][pos.GetIndex()]) {
      // There's not actually a taxon here
      return;
    }

    RemoveOrgAfterRepro(taxon_locations[pos.GetPopID()][pos.GetIndex()]);
    removal_pos = pos;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrgAfterRepro(Ptr<taxon_t> taxon) {
    if (to_be_removed != nullptr) {
      RemoveOrg(to_be_removed);
      taxon_locations[removal_pos.GetPopID()][removal_pos.GetIndex()] = nullptr;
      to_be_removed = nullptr;
      removal_pos = {0, 0};
    }
    to_be_removed = taxon;
  }


  // Remove an instance of a taxon; track when it's gone.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  bool Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrg(WorldPosition pos) {
    emp_assert(store_position, "Trying to remove org based on position from systematics manager that doesn't track it.");
    emp_assert(pos.GetPopID() < taxon_locations.size(), "Invalid population requested for removal", pos.GetPopID(), taxon_locations.size());
    emp_assert(pos.GetIndex() < taxon_locations[pos.GetPopID()].size(), "Invalid position requested for removal", pos.GetIndex(), taxon_locations[pos.GetPopID()].size());

    bool active = false;
    if (taxon_locations[pos.GetPopID()][pos.GetIndex()]) {
      //TODO: Figure out how this can ever not be true
      active = RemoveOrg(taxon_locations[pos.GetPopID()][pos.GetIndex()]);
    }
    taxon_locations[pos.GetPopID()][pos.GetIndex()] = nullptr;
    return active;
  }

  // Remove an instance of a taxon; track when it's gone.
  // @param taxon the taxon of which one instance is being removed
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  bool Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrg(Ptr<taxon_t> taxon) {
    emp_assert(taxon);

    // Update stats
    org_count--;
    total_depth -= taxon->GetDepth();

    // emp_assert(Has(active_taxa, taxon));
    const bool active = taxon->RemoveOrg();
    if (!active) MarkExtinct(taxon);

    return active;
  }

  // Remove all taxa that 1) went extinct before the specified update/time step,
  // and 2) only have ancestors that went extinct before the specified update/time step.
  // Warning: this function invalidates most measurements you could make about tree topology.
  // It is useful in select situations where you need to store ancestors for some period of time,
  // but cannot computationally afford to store all ancestors for your entire run.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveBefore(int ud) {

    std::set<Ptr<taxon_t>> to_remove;
    for (Ptr<taxon_t> tax : ancestor_taxa) {
      if (tax->GetDestructionTime() < ud && CanRemove(tax, ud)) {
        to_remove.insert(tax);
      }
    }

    for (Ptr<taxon_t> tax : to_remove) {
        for (Ptr<taxon_t> off : tax->GetOffspring()) {
          off->NullifyParent();
        }
        ancestor_taxa.erase(tax);
        tax.Delete();
    }

  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  /// Helper function for RemoveBefore
  /// @returns true if a a taxon can safely be
  /// removed by RemoveBefore
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  bool Systematics<ORG, ORG_INFO, DATA_STRUCT>::CanRemove(Ptr<taxon_t> t, int ud) {
    if (!t) {
      return false;
    }
    while (t) {
      if (t->GetNumOrgs() > 0 || t->GetDestructionTime() >= ud) {
        return false;
      }
      t = t->GetParent();
    }
    return true;
  }
  #endif // #DOXYGEN_SHOULD_SKIP_THIS

  // ======= Functions for getting information from the systematics manager

  // @returns a pointer to the parent of a given taxon
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t> Systematics<ORG, ORG_INFO, DATA_STRUCT>::Parent(Ptr<taxon_t> taxon) const {
    emp_assert(taxon);
    emp_assert(Has(active_taxa, taxon));
    return taxon->GetParent();
  }

  // Print details about the Systematics manager.
  // First prints setting, followed by all active, ancestor, and outside
  // taxa being stored. Format for taxa is
  // [ id | number of orgs in this taxon, number of offspring taxa of this taxon | parent taxon]
  // @param os output stream to print to
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::PrintStatus(std::ostream & os) const {
    os << "Systematics Status:\n";
    os << " store_active=" << store_active
       << " store_ancestors=" << store_ancestors
       << " store_outside=" << store_outside
       << " archive=" << archive
       << " next_id=" << next_id
       << " synchronous=" << track_synchronous
       << std::endl;
    os << "Active count:   " << active_taxa.size();
    for (const auto & x : active_taxa) {
      os << " [" << x->GetID() << "|" << x->GetNumOrgs() << "," << x->GetNumOff() << "|"
         << (x->GetParent() ? emp::to_string(x->GetParent()->GetID()) : "null") << "]";
    }
    os << std::endl;

    os << "Ancestor count: " << ancestor_taxa.size();
    for (const auto & x : ancestor_taxa) {
      os << " [" << x->GetID() << "|" << x->GetNumOrgs() << "," << x->GetNumOff() << "|"
         << (x->GetParent() ? emp::to_string(x->GetParent()->GetID()) : "null") << "]";
    }
    os << std::endl;

    os << "Outside count:  " << outside_taxa.size();
    for (const auto & x : outside_taxa) {
      os << " [" << x->GetID() << "|" << x->GetNumOrgs() << "," << x->GetNumOff() << "|"
         << (x->GetParent() ? emp::to_string(x->GetParent()->GetID()) : "null") << "]";
    }
    os << std::endl;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::PrintLineage(Ptr<taxon_t> taxon, std::ostream & os) const {
    os << "Lineage:\n";
    while (taxon) {
      os << taxon->GetInfo() << std::endl;
      taxon = taxon->GetParent();
    }
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::Snapshot(const std::string & file_path) const {
    emp::DataFile file(file_path);
    Ptr<taxon_t> cur_taxon;
    emp::vector<std::function<std::string()>> wrapped_user_funs;
    // Add default functions to file.
    //  - id: systematics ID for taxon
    std::function<size_t()> get_id = [&cur_taxon]() {
      return cur_taxon->GetID();
    };
    file.AddFun(get_id, "id", "Systematics ID for this taxon.");

    //  - ancestor_list: ancestor list for taxon
    std::function<std::string()> get_ancestor_list = [&cur_taxon]() -> std::string {
      if (cur_taxon->GetParent() == nullptr) { return "[\"NONE\"]"; }
      return "[" + to_string(cur_taxon->GetParent()->GetID()) + "]";
    };
    file.AddFun(get_ancestor_list, "ancestor_list", "Ancestor list for this taxon.");

    //  - origin_time: When did this taxon first appear in the population?
    std::function<double()> get_origin_time = [&cur_taxon]() {
      return cur_taxon->GetOriginationTime();
    };
    file.AddFun(get_origin_time, "origin_time", "When did this taxon first appear in the population?");

    //  - destruction_time: When did this taxon leave the population?
    std::function<double()> get_destruction_time = [&cur_taxon]() {
      return cur_taxon->GetDestructionTime();
    };
    file.AddFun(get_destruction_time, "destruction_time", "When did this taxon leave the population?");

    //  - num_orgs: How many organisms currently exist of this group?
    std::function<size_t()> get_num_orgs = [&cur_taxon]() {
      return cur_taxon->GetNumOrgs();
    };
    file.AddFun(get_num_orgs, "num_orgs", "How many organisms currently exist of this group?");

    //  - tot_orgs: How many organisms have ever existed of this group?
    std::function<size_t()> get_tot_orgs = [&cur_taxon]() {
      return cur_taxon->GetTotOrgs();
    };
    file.AddFun(get_tot_orgs, "tot_orgs", "How many organisms have ever existed of this group?");

    //  - num_offspring: How many direct offspring groups exist from this one.
    std::function<size_t()> get_num_offspring = [&cur_taxon]() {
      return cur_taxon->GetNumOff();
    };
    file.AddFun(get_num_offspring, "num_offspring", "How many direct offspring groups exist from this one.");

    //  - total_offspring: How many total extant offspring taxa exist from this one (i.e. including indirect)
    std::function<size_t()> get_total_offspring = [&cur_taxon]() {
      return cur_taxon->GetTotalOffspring();
    };
    file.AddFun(get_total_offspring, "total_offspring", "How many total extant offspring taxa exist from this one (i.e. including indirect)");

    //  - depth: How deep in tree is this node? (Root is 0)
    std::function<size_t()> get_depth = [&cur_taxon]() {
      return cur_taxon->GetDepth();
    };
    file.AddFun(get_depth, "depth", "How deep in tree is this node? (Root is 0)");

    // Add user-added functions to file.
    for (size_t i = 0; i < user_snapshot_funs.size(); ++i) {
      wrapped_user_funs.emplace_back([this, i, &cur_taxon]() -> std::string {
        return user_snapshot_funs[i].fun(*cur_taxon);
      });
    }

    // Need to add file functions after wrapping to preserve integrity of
    // function reference being passed to the data file object.
    for (size_t i = 0; i < user_snapshot_funs.size(); ++i) {
      file.AddFun(wrapped_user_funs[i], user_snapshot_funs[i].key, user_snapshot_funs[i].desc);
    }

    // Output header information.
    file.PrintHeaderKeys();

    // Update file w/ taxa information
    // const auto all = {std::ranges::ref_view(active_taxa), std::ranges::ref_view(ancestor_taxa),
                      // std::ranges::ref_view(outside_taxa)};
    for (emp::Ptr<taxon_t> tax : active_taxa) {
      cur_taxon = tax;
      file.Update();
    }
    for (emp::Ptr<taxon_t> tax : ancestor_taxa) {
      cur_taxon = tax;
      file.Update();
    }
    for (emp::Ptr<taxon_t> tax : outside_taxa) {
      cur_taxon = tax;
      file.Update();
    }

  }

  // ======= Measurements about the systematics manager

  // @returns the genetic diversity of the population.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  double Systematics<ORG, ORG_INFO, DATA_STRUCT>::CalcDiversity() const {
    return emp::Entropy(active_taxa, [](Ptr<taxon_t> x){ return x->GetNumOrgs(); }, (double) org_count);
  }

  // @returns a pointer to the Most-Recent Common Ancestor for the population or null pointer if there isn't one
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t> Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetMRCA() const {
    if (!mrca && num_roots == 1) {  // Determine if we need to calculate the MRCA.
      // First, find a candidate among the living taxa.  Only taxa that have one offsrping
      // can be on the line-of-descent to the MRCA, so anything else is a good start point.
      // There must be at least one!  Stop as soon as we find a candidate.
      Ptr<taxon_t> candidate(nullptr);
      for (auto x : active_taxa) {
        if (x->GetNumOff() != 1) { candidate = x; break; }
      }

      // Now, trace the line of descent, updating the candidate as we go.
      Ptr<taxon_t> test_taxon = candidate->GetParent();
      while (test_taxon) {
        emp_assert(test_taxon->GetNumOff() >= 1);
        // If the test_taxon is dead, we only want to update candidate when we hit a new branch point
        // If test_taxon is still alive, though, we always need to update it
        if (test_taxon->GetNumOff() > 1 || test_taxon->GetNumOrgs() > 0) candidate = test_taxon;
        test_taxon = test_taxon->GetParent();
      }
      mrca = candidate;
    }
    return mrca;
  }

  // @returns the depth of the Most-Recent Common Ancestor or -1 for none.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  int Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetMRCADepth() const {
    GetMRCA();
    if (mrca) return (int) mrca->GetDepth();
    return -1;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t> Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetSharedAncestor(Ptr<taxon_t> t1, Ptr<taxon_t> t2) const {
    // Same taxon
    if (t1 == t2) {
      return t1;
    }

    // If not same, we have to actually do work
    emp::vector<Ptr<taxon_t> > lineage1 = GetLineageToMRCA(t1);
    emp::vector<Ptr<taxon_t> > lineage2 = GetLineageToMRCA(t2);

    size_t l1 = lineage1.size() - 1;
    size_t l2 = lineage2.size() - 1;

    emp_assert(lineage1[l1] == lineage2[l2],
        "Both lineages should start with MRCA");

    while (lineage1[l1] == lineage2[l2]) {
      l1--;
      l2--;
    }

    return lineage1[l1+1];
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  // Helper for Colless function calculation
  struct CollessStruct {
    double total = 0;
    emp::vector<double> ns;
  };

  // Helper for Colless function calculation
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  CollessStruct Systematics<ORG, ORG_INFO, DATA_STRUCT>::RecursiveCollessStep(Ptr<taxon_t> curr) const {
    CollessStruct result;

    while (curr->GetNumOff() == 1) {
      curr = *(curr->GetOffspring().begin());
    }

    if (curr->GetNumOff() == 0) {
      result.ns.push_back(0); // Node itself is calculated at level above
      return result;
    }

    for (Ptr<taxon_t> off : curr->GetOffspring()) {
      // std::cout << "Recursing on ID: " << off->GetID() << " Offspring: " << off->GetTotalOffspring() << std::endl;

      CollessStruct new_result = RecursiveCollessStep(off);
      result.ns.push_back(Sum(new_result.ns) + log(off->GetOffspring().size() + exp(1)));
      result.total += new_result.total;
    }

    // std::cout << "Evaluating: " << curr->GetID() << std::endl;

    double med = Median(result.ns);
    double sum_diffs = 0;
    // std::cout << "Median: " << med << std::endl;
    for (double n : result.ns) {
      // std::cout << n << std::endl;
      sum_diffs += std::abs(n-med);
    }
    // std::cout << "Sumdiffs: " << sum_diffs << " n: " << result.ns.size() << " average: " << sum_diffs/result.ns.size() << std::endl;
    result.total += sum_diffs/result.ns.size();
    return result;
  }
  #endif // #DOXYGEN_SHOULD_SKIP_THIS

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  emp::vector<double> Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetPairwiseDistances(bool branch_only) const {
    // The overarching approach here is to start with a bunch of pointers to all
    // extant organisms (since that will include all leaves). Then we trace back up
    // the tree, keeping track of distances. When things meet up, we calculate
    // distances between the nodes on the sides that just met up.

    emp::vector<double> dists;

    std::map< Ptr<taxon_t>, emp::vector<emp::vector<int>> > curr_pointers;
    std::map< Ptr<taxon_t>, emp::vector<emp::vector<int>> > next_pointers;


    for (Ptr<taxon_t> tax : active_taxa) {
      curr_pointers[tax] = emp::vector<emp::vector<int>>({{0}});
    }

    // std::cout << "Starting curr_pointers size: " << curr_pointers.size() << std::endl;

    while (curr_pointers.size() > 0) {
      for (auto & tax : curr_pointers) {
        bool alive = tax.first->GetNumOrgs() > 0;
        // std::cout << tax.first << " has " << to_string(tax.second) << "and is waiting for " << tax.first->GetNumOff() + int(alive) << std::endl;
        if ( tax.second.size() < tax.first->GetNumOff() + int(alive)) {
          if (Has(next_pointers, tax.first)) {
            // In case an earlier iteration added this node to next_pointers
            for (auto vec : tax.second) {
              next_pointers[tax.first].push_back(vec);
            }
          } else {
            next_pointers[tax.first] = curr_pointers[tax.first];
          }
          continue;
        }
        emp_assert(tax.first->GetNumOff() + int(alive) == tax.second.size(), tax.first->GetNumOff(), alive, to_string(tax.second), tax.second.size());

        // Okay, things should have just met up. Let's compute the distances
        // between everything that just met.

        if (tax.second.size() > 1) {

          for (size_t i = 0; i < tax.second.size(); i++ ) {
            for (size_t j = i+1; j < tax.second.size(); j++) {
              for (int disti : tax.second[i]) {
                for (int distj : tax.second[j]) {
                  // std::cout << "Adding " << disti << " and " << distj << std::endl;
                  dists.push_back(disti+distj);
                }
              }
            }
          }
        }
        // std::cout << "dists " << to_string(dists) << std::endl;
        // Increment distances and stick them in new vector
        emp::vector<int> new_dist_vec;
        for (auto & vec : tax.second) {
          for (int el : vec) {
            new_dist_vec.push_back(el+1);
          }
        }

        // std::cout << "new_dist_vec " << to_string(new_dist_vec) << std::endl;

        next_pointers.erase(tax.first);

        Ptr<taxon_t> test_taxon = tax.first->GetParent();
        while (test_taxon && test_taxon->GetNumOff() == 1 && test_taxon->GetNumOrgs() == 0) {
          if (!branch_only) {
            for (size_t i = 0; i < new_dist_vec.size(); i++){
              new_dist_vec[i]++;
            }
          }
          test_taxon = test_taxon->GetParent();
        }

        if (!test_taxon) {
          continue;
        } else if (!Has(next_pointers, test_taxon)) {
          next_pointers[test_taxon] = emp::vector<emp::vector<int> >({new_dist_vec});
        } else {
          next_pointers[test_taxon].push_back(new_dist_vec);
        }
      }
      curr_pointers = next_pointers;
      next_pointers.clear();
      // std::cout << curr_pointers.size() << std::endl;
    }

    if (dists.size() != (active_taxa.size()*(active_taxa.size()-1))/2) {
      // The tree is not connected
      // It's possible we should do something different here...
      return dists;
    }

    // std::cout << "Total: " << total << "Dists: " << dists.size() << std::endl;

    return dists;

  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  double Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetEvolutionaryDistinctiveness(Ptr<taxon_t> tax, double time) const {

    double depth = 0; // Length (in time units) of section we're currently exploring
    double total = 0; // Count up scores for each section of tree
    double divisor = tax->GetTotalOffspring() + 1; // Number of extant taxa this will split into (1 for current taxa, plus its offspring)

    // We're stopping when we hit MRCA, so we need to make sure it's been calculated.
    GetMRCA();
    if (tax == mrca) {
      return 0;
    }

    // std::cout << "Initializing divisor to " << divisor << " Offspring: " << tax->GetTotalOffspring() << std::endl;
    // std::cout << "MRCA ID: " << mrca->GetID() << " Tax ID: " << tax->GetID() << " time: " << time << " Orig: " << tax->GetOriginationTime() << std::endl;

    Ptr<taxon_t> test_taxon = tax->GetParent();

    emp_assert(time != -1 && "Invalid time - are you passing time to rg?", time);
    emp_assert(time >= tax->GetOriginationTime()
                && "GetEvolutionaryDistinctiveness received a time that is earlier than the taxon's origination time.", tax->GetOriginationTime(), time);

    while (test_taxon) {

      // emp_assert(test_taxon->GetOriginationTime() != -1 &&
      //           "Invalid time - are you passing time to rg?", time);

      depth += time - test_taxon->GetOriginationTime();
      // std::cout << "Tax: " << test_taxon->GetID() << " depth: " << depth << " time: " << time  << " Orig: " << test_taxon->GetOriginationTime() << " divisor: " << divisor << std::endl;
      time = test_taxon->GetOriginationTime();
      if (test_taxon == mrca || !test_taxon) {
        // Stop when everything has converged or when we hit the root.
        // std::cout << (int)(test_taxon == mrca) << " depth: " << depth << " divisor: " << divisor << std::endl;
        total += depth/divisor;
        return total;
      } else if (test_taxon->GetNumOrgs() > 0) {
        // If this taxon is still alive we need to update the divisor
        // std::cout << "Alive point" << " depth: " << depth << " divisor: " << divisor << std::endl;
        total += depth/divisor;
        depth = 0;
        divisor = test_taxon->GetTotalOffspring() + 1;
      } else if (test_taxon->GetNumOff() > 1) {
        // This is a branch point. We need to add the things on the other branch to the divisor..
        // std::cout << "Branch point" << " depth: " << depth << " divisor: " << divisor << std::endl;
        total += depth/divisor;
        depth = 0;
        divisor = test_taxon->GetTotalOffspring();
      }

      test_taxon = test_taxon->GetParent();
    }

    return -1;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  int Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetBranchesToRoot(Ptr<taxon_t> tax) const {
    GetMRCA();

    int depth = 0;
    Ptr<taxon_t> test_taxon = tax->GetParent();
    while (test_taxon) {
      if (test_taxon == mrca || !test_taxon) {
        return depth;
      } else if (test_taxon->GetNumOff() > 1) {
        depth++;
      }
      test_taxon = test_taxon->GetParent();
    }
    return depth;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  int Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetDistanceToRoot(Ptr<taxon_t> tax) const {
    // Now, trace the line of descent, updating the candidate as we go.
    GetMRCA();

    int depth = 0;
    Ptr<taxon_t> test_taxon = tax->GetParent();
    while (test_taxon) {
      depth++;
      if (test_taxon == mrca || !test_taxon) {
        return depth;
      }
      test_taxon = test_taxon->GetParent();
    }
    return depth;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  std::set<Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>> Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetCanopyExtantRoots(int time_point) const {
    // NOTE: This could be made faster by doing something similar to the pairwise distance
    // function
    using taxon_t = Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t;
    std::set< Ptr<taxon_t>> result;
    // std::cout << "starting " << time_point << std::endl;
    for (Ptr<taxon_t> tax : active_taxa) {
        // std::cout << tax->GetInfo() << std::endl;
      while (tax) {
        // std::cout << tax->GetInfo() << " " << tax->GetOriginationTime() << " " << tax->GetDestructionTime() << std::endl;
        if (tax->GetOriginationTime() <= time_point && tax->GetDestructionTime() > time_point ) {
          result.insert(tax);
        // std::cout << "inserting " << tax->GetInfo() << std::endl;
          break;
        }
        tax = tax->GetParent();
      }
    }

    return result;

  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  int Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetPhylogeneticDiversityNormalize(int generation, std::string filename) const {
    int gen_value = ((generation / 10) - 1); //indexes from 0, 100 generations would correspond to the 10th line in the csv
    // bool percent_found = false;
    int phylogenetic_diversity = ancestor_taxa.size() + active_taxa.size() - 1;

    if(filename == ""){
      //std::cout << "Phylogenetic Diversity is " << phylogenetic_diversity << std::endl;
      return phylogenetic_diversity;
    } else{

      emp::File generation_percentiles(filename); //opens file
      emp::vector< emp::vector<double> >percentile_data = generation_percentiles.ToData<double>(','); //turns file contents into vector

      for(int j = 0; j <= percentile_data[gen_value].size() - 2; j++){ //searches through vector for slot where phylo diversity fits

        if((percentile_data[gen_value][j] <= phylogenetic_diversity) && (percentile_data[gen_value][j + 1] > phylogenetic_diversity)){
          // std::cout << "phylogenetic diversity is in between: " << percentile_data[gen_value][j] << "and " << percentile_data[gen_value][j+1] << std::endl;
          // std::cout << "The phylogenetic diversity value " << phylogenetic_diversity << " is in the " << j << " percentile, in the " << ((gen_value + 1)* 10) << " generation" << std::endl;
          return j;
        }
      }
    }
    return 100;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  int Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetMaxDepth() {
    if (max_depth != -1) {
      return max_depth;
    }

    for (auto tax : active_taxa) {
      int depth = tax->GetDepth();
      if (depth > max_depth) {
        max_depth = depth;
      }
    }
    return max_depth;
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::LoadFromFile(const std::string & file_path,
                                                            const std::string & info_col,
                                                             bool assume_leaves_extant) {

    // We can only load phylogenies from file if their info can be
    // converted to this systematics object's ORG_INFO type (if you
    // have a complex type, you can just use a string representation)
    if constexpr (!emp::is_streamable<std::stringstream, ORG_INFO>::value) {
      emp_assert(false && "Failed to load phylogeny from file. ORG_INFO template type cannot be created from string");
      return;
    }

    // Load files
    emp::File in_file(file_path);
    in_file.RemoveWhitespace();
    emp::vector<std::string> header = in_file.ExtractRow();

    // Find column ids
    auto id_pos_it = std::find(header.begin(), header.end(), "id");
    emp_assert(id_pos_it != header.end() &&
      "Input phylogeny file must be in ALife Phylogeny Data Standards format" &&
      "id column is missing");
    size_t id_pos = std::distance(header.begin(), id_pos_it);

    auto anc_pos_it = std::find(header.begin(), header.end(), "ancestor_list");
    emp_assert(anc_pos_it != header.end() &&
      "Input phylogeny file must be in ALife Phylogeny Data Standards format" &&
      "ancestor_list column is missing");
    size_t anc_pos = std::distance(header.begin(), anc_pos_it);

    auto origin_pos_it = std::find(header.begin(), header.end(), "origin_time");
    int origin_pos = -1;
    if(origin_pos_it != header.end()){
      origin_pos = std::distance(header.begin(), origin_pos_it);
    }

    auto destruction_pos_it = std::find(header.begin(), header.end(), "destruction_time");
    int destruction_pos = -1;
    if (destruction_pos_it != header.end()) {
      destruction_pos = std::distance(header.begin(), destruction_pos_it);
    }

    auto num_orgs_pos_it = std::find(header.begin(), header.end(), "num_orgs");
    int num_orgs_pos = -1;
    if (num_orgs_pos_it != header.end()) {
      num_orgs_pos = std::distance(header.begin(), num_orgs_pos_it);
    }

    auto tot_orgs_pos_it = std::find(header.begin(), header.end(), "tot_orgs");
    int tot_orgs_pos = -1;
    if (tot_orgs_pos_it != header.end()) {
      tot_orgs_pos = std::distance(header.begin(), tot_orgs_pos_it);
    }

    auto info_pos_it = std::find(header.begin(), header.end(), info_col);
    emp_assert(info_pos_it != header.end() &&
      "Input phylogeny file must be in ALife Phylogeny Data Standards format" &&
      "info column name supplied is not in file.");
    size_t info_pos = std::distance(header.begin(), info_pos_it);

    // Keep track taxon objects
    std::unordered_map<int, emp::Ptr<taxon_t> > taxa;
    // File is out of order, so we have to link up parents
    // and offspring after the fact
    std::unordered_map<int, int> unlinked_parents;
    // Keep track of roots
    emp::vector<emp::Ptr<taxon_t>> roots;

    // Read in each row and make a taxon for it
    size_t num_lines = in_file.GetNumLines();
    for (size_t i = 0; i < num_lines; i++) {
      emp::vector<std::string> row = in_file.ExtractRow();
      int id = emp::from_string<int>(row[id_pos]);

      // Inf means this taxon is still alive
      // or we don't know which taxa are alive
      std::string destruction_time = "inf";
      if (destruction_pos != -1) {
        destruction_time = row[destruction_pos];
      } else {
        destruction_time = "unknown";
      }

      // Load ancestor list
      ORG_INFO info = emp::from_string<ORG_INFO>(row[info_pos]);
      std::string ancestor_list_str = row[anc_pos];
      emp::remove_chars(ancestor_list_str, "[]\"");
      emp::Ptr<taxon_t> tax;

      // Make taxon (parent is nullptr for now)
      tax.New(id, info);
      if (destruction_time != "inf") {
        ancestor_taxa.insert(tax);
      } else {
        active_taxa.insert(tax);
      }

      // Fill in destruction and origin time if
      // provided
      if (origin_pos != -1 ){
        double origin_time = emp::from_string<double>(row[origin_pos]);
        tax->SetOriginationTime(origin_time);
      }
      if (destruction_time != "inf" && destruction_time != "unknown") {
        tax->SetDestructionTime(emp::from_string<double>(destruction_time));
      }

      // Fill in number of current and total orgs if provided
      if (num_orgs_pos != -1) {
        size_t num_orgs = emp::from_string<size_t>(row[num_orgs_pos]);
        tax->SetNumOrgs(num_orgs);
      }
      if (tot_orgs_pos != -1) {
        size_t tot_orgs = emp::from_string<size_t>(row[tot_orgs_pos]);
        tax->SetTotOrgs(tot_orgs);
      }

      // Store taxon pointer
      taxa[id] = tax;
      // Keep track of parents so we can link up later
      if (emp::to_lower(ancestor_list_str) != "none") {
        emp::vector<std::string> ancestor_split = emp::slice(ancestor_list_str, ',');
        emp::vector<int> ancestor_list = emp::from_strings<int>(ancestor_split);
        unlinked_parents[id] = ancestor_list[0];
      } else {
        // If no parent, this is a root
        num_roots++;
        roots.push_back(tax);
      }
    }

    // Link up parents and offspring
    for (auto element : unlinked_parents) {
      taxa[element.first]->parent = taxa[element.second];
      taxa[element.second]->AddOffspring(taxa[element.first]);
    }

    // Set up depth
    emp::vector<emp::Ptr<taxon_t>> leaves;
    emp::vector<emp::Ptr<taxon_t>> to_explore;
    for (auto root : roots) {
      root->depth = 0;
      root->total_offspring = 0;
      for (auto offspring : root->GetOffspring()) {
        to_explore.push_back(offspring);
      }
    }

    // Step through all taxa and fix their
    // bookkeeping
    emp::Ptr<taxon_t> curr;
    while(!to_explore.empty()) {
      curr = to_explore.back();
      to_explore.pop_back();
      curr->total_offspring = 0;
      curr->depth = curr->GetParent()->depth + 1;
      for (auto offspring : curr->GetOffspring()){
        to_explore.push_back(offspring);
      }
      if (curr->GetNumOff() == 0) {
        leaves.push_back(curr);
      }
    }

    // If we're assuming that all leave are extant,
    // move leaves to active taxa
    for (auto leaf : leaves) {
      if (assume_leaves_extant && !Has(active_taxa, leaf)) {
        ancestor_taxa.erase(leaf);
        active_taxa.insert(leaf);
      }
    }

    // Adjust total offspring
    for (auto tax : active_taxa) {
      tax->parent->AddTotalOffspring();
    }

    // Force stats to be recalculated
    max_depth = -1;
    mrca = nullptr;

  }

}

#endif // #ifndef EMP_EVOLVE_SYSTEMATICS_HPP_INCLUDE
