/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  Systematics.hpp
 *  @brief Track genotypes, species, clades, or lineages of organisms in a world.
 *
 *
 *  @todo Technically, we don't need to keep the ancestors in a set in order to track a lineage...
 *        If we delete all of their descendants they should automaticaly be deleted.
 *  @todo We should provide an option to back up systematics data to a file so that it doesn't all
 *        need to be kept in memory, especially if we're only doing post-analysis.
 *  @todo This inheritance system makes adding new systematics-related data tracking kind of a pain.
 *        Over time, this will probably become a maintainability problem. We can probably make the
 *        whole inheritance thing go away through judicious use of signals.
 * @todo This does not currently handle situations where organisms change locations during their
 *       lifetimes gracefully.
 */


#ifndef EMP_EVO_SYSTEMATICS_H
#define EMP_EVO_SYSTEMATICS_H

#include <ostream>
#include <set>
#include <unordered_set>
#include <map>
#include <limits>

#include "../base/Ptr.hpp"
#include "../control/Signal.hpp"
#include "../data/DataManager.hpp"
#include "../data/DataNode.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/set_utils.hpp"
#include "../math/info_theory.hpp"
#include "../math/stats.hpp"
#include "../tools/string_utils.hpp"
#include "../data/DataFile.hpp"
#include "SystematicsAnalysis.hpp"
#include "World_structure.hpp"

namespace emp {

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

        const double GetFitness() const {
          return fitness.GetMean();
        }
    };

    template <typename PHEN_TYPE>
    struct mut_landscape_info { /// Track information related to the mutational landscape
      /// Maps a string representing a type of mutation to a count representing
      /// the number of that type of mutation that occurred to bring about this taxon.
      using phen_t = PHEN_TYPE;
      using has_phen_t = std::true_type;
      using has_mutations_t = std::true_type;
      using has_fitness_t = std::true_type;
      // using has_phenotype_t = true;

      std::unordered_map<std::string, int> mut_counts;
      DataNode<double, data::Current, data::Range> fitness; /// This taxon's fitness (for assessing deleterious mutational steps)
      PHEN_TYPE phenotype; /// This taxon's phenotype (for assessing phenotypic change)

      const PHEN_TYPE & GetPhenotype() const {
        return phenotype;
      }

      const double GetFitness() const {
        return fitness.GetMean();
      }

      void RecordMutation(std::unordered_map<std::string, int> muts) {
        for (auto mut : muts) {
          if (Has(mut_counts, mut.first)) {
            mut_counts[mut.first] += mut.second;
          } else {
            mut_counts[mut.first] = mut.second;
          }
        }
      }

      void RecordFitness(double fit) {
        fitness.Add(fit);
      }

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

    /// Get the total number of organisms that have ever lived associated with this Taxon
    size_t GetTotOrgs() const { return tot_orgs; }

    /// Get the number of taxa that were produced by organisms from this Taxon.
    size_t GetNumOff() const { return num_offspring; }

    /// Get the number of taxanomic steps since the ancestral organism was injected into the World.
    size_t GetDepth() const { return depth; }

    data_t & GetData() {return data;}
    const data_t & GetData() const {return data;}

    std::set<Ptr<this_t> > GetOffspring() {return offspring;}

    void SetData(data_t d) {data = d;}

    double GetOriginationTime() const {return origination_time;}
    void SetOriginationTime(double time) {origination_time = time;}

    double GetDestructionTime() const {return destruction_time;}
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
  /// and providing virtual functions.

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
    size_t num_roots;         ///< How many distint injected ancestors are currently in population?
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

    // Returns a reference so that capturing it in a lambda to call on update
    // is less confusing. It's possible we should change it to be consistent
    // with GetFitnessDataNode, though.
    data_ptr_t AddDataNode(const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      return &(data_nodes.New(name));
    }

    data_ptr_t AddDataNode(std::function<emp::vector<double>()> pull_set_fun, const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      auto node = AddDataNode(name);
      node->AddPullSet(pull_set_fun);
      return node;
    }

    data_ptr_t AddDataNode(std::function<double()> pull_fun, const std::string & name) {
      emp_assert(!data_nodes.HasNode(name));
      auto node = AddDataNode(name);
      node->AddPull(pull_fun);
      return node;
    }


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
    virtual int GetMRCADepth() const = 0;
    virtual void AddOrg(ORG && org, WorldPosition pos, int update) = 0;
    virtual void AddOrg(ORG & org, WorldPosition pos, int update) = 0;
    virtual bool RemoveOrg(WorldPosition pos, int time=-1) = 0;
    virtual void RemoveOrgAfterRepro(WorldPosition pos, int time=-1) = 0;
    // virtual bool RemoveNextOrg(WorldPosition pos, int time=-1) = 0;
    virtual void PrintStatus(std::ostream & os) const = 0;
    virtual double CalcDiversity() const = 0;
    virtual void Update() = 0;
    virtual void SetNextParent(int pos) = 0;
    virtual void SetNextParent(WorldPosition & pos) = 0;
  };

  /// @brief A tool to track phylogenetic relationships among organisms.
  /// The systematics class tracks the relationships among all organisms based on the INFO_TYPE
  /// provided.  If an offspring has the same value for INFO_TYPE as its parent, it is grouped into
  /// the same taxon.  Otherwise a new Taxon is created and the old one is used as its parent in
  /// the phylogeny.  If the provided INFO_TYPE is the organsism's genome, a traditional phylogeny
  /// is formed, with genotypes.  If the organism's behavior/task set is used, then organisms are
  /// grouped by phenotypes.  If the organsims's position is used, the evolutionary path through
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

    fun_calc_info_t calc_info_fun;
    Ptr<taxon_t> next_parent;
    Ptr<taxon_t> most_recent;

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
    // using parent_t::OnNew;
    // using parent_t::OnPrune;
    using parent_t::GetPhylogeneticDiversity;
    // using parent_t::GetTaxonDistinctiveness;
    // using parent_t::GetEvolutionaryDistinctiveness;
    using parent_t::GetMeanPairwiseDistance;
    using parent_t::GetSumPairwiseDistance;
    using parent_t::GetVariancePairwiseDistance;
    using parent_t::GetPairwiseDistances;
    // using parent_t::GetDistanceToRoot;
    // using parent_t::GetBranchesToRoot;
    // using parent_t::GetMRCA;
    using parent_t::GetMRCADepth;
    using parent_t::AddOrg;
    using parent_t::RemoveOrg;
    using parent_t::RemoveOrgAfterRepro;
    // using parent_t::RemoveNextOrg;
    // using parent_t::Parent;
    using parent_t::PrintStatus;
    // using parent_t::PrintLineage;
    using parent_t::CalcDiversity;
    using parent_t::Update;
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

    struct SnapshotInfo {
      using snapshot_fun_t = std::function<std::string(const taxon_t &)>;
      snapshot_fun_t fun;
      std::string key;
      std::string desc;

      SnapshotInfo(const snapshot_fun_t & _fun, const std::string & _key, const std::string & _desc="")
        : fun(_fun),
          key(_key),
          desc(_desc)
      { ; }
    };

    emp::vector<SnapshotInfo> user_snapshot_funs;

    std::unordered_set< Ptr<taxon_t>, hash_t > active_taxa;   ///< A set of all living taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > ancestor_taxa; ///< A set of all dead, ancestral taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > outside_taxa;  ///< A set of all dead taxa w/o descendants.

    Ptr<taxon_t> to_be_removed = nullptr;
    int removal_time = -1;
    int removal_pos = -1;

    emp::vector<Ptr<taxon_t> > taxon_locations;
    emp::vector<Ptr<taxon_t> > next_taxon_locations;

    Signal<void(Ptr<taxon_t>, ORG & org)> on_new_sig; ///< Trigger when any organism is pruned from tree
    Signal<void(Ptr<taxon_t>)> on_prune_sig; ///< Trigger when any organism is pruned from tree

    mutable Ptr<taxon_t> mrca;  ///< Most recent common ancestor in the population.

    /// Called wheneven a taxon has no organisms AND no descendants.
    void Prune(Ptr<taxon_t> taxon);

    /// Called when an offspring taxa has been deleted.
    void RemoveOffspring(Ptr<taxon_t> offspring, Ptr<taxon_t> taxon);

    /// Called when there are no more living members of a taxon.  There may be descendants.
    void MarkExtinct(Ptr<taxon_t> taxon, int time=-1);



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


    void Update() {
      ++curr_update;
      if (track_synchronous) {

        // Clear pending removal
        if (to_be_removed != nullptr) {
          RemoveOrg(to_be_removed, removal_time);
          taxon_locations[removal_pos] = nullptr;
          to_be_removed = nullptr;
          removal_pos = -1;
        }

        std::swap(taxon_locations, next_taxon_locations);
        next_taxon_locations.resize(0);
      }
    }

    void SetCalcInfoFun(fun_calc_info_t f) {calc_info_fun = f;}

    // Currently using raw pointers because of a weird bug in emp::Ptr. Should switch when fixed.
    std::unordered_set< Ptr<taxon_t>, hash_t > * GetActivePtr() { return &active_taxa; }
    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetActive() const { return active_taxa; }
    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetAncestors() const { return ancestor_taxa; }
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

    int GetMaxDepth() {
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

    void SetNextParent(WorldPosition & pos) {
      emp_assert(pos.IsActive() || !pos.IsValid());
      if (!pos.IsValid()) {
        next_parent = nullptr;
      } else {
        next_parent = taxon_locations[pos.GetIndex()];
      }
    }

    void SetNextParent(int pos) {
      emp_assert(pos < (int)taxon_locations.size(), "Invalid parent", pos, taxon_locations.size());
      if (pos == -1) {
        next_parent = nullptr;
      } else {
        emp_assert(pos >= 0, "Invalid parent", pos);
        emp_assert(taxon_locations[pos], pos);
        next_parent = taxon_locations[pos];
      }
    }

    void SetNextParent(Ptr<taxon_t> p) {
      next_parent = p;
    }

    Ptr<taxon_t> GetNextParent() {
      return next_parent;
    }

    Ptr<taxon_t> GetMostRecent() {
      return most_recent;
    }

    SignalKey OnNew(std::function<void(Ptr<taxon_t>, ORG & org)> & fun) { return on_new_sig.AddAction(fun); }

    /// Privide a function for Systematics to call each time a taxon is about to be pruned.
    /// Trigger:  Taxon is about to be killed
    /// Argument: Pointer to taxon
    SignalKey OnPrune(std::function<void(Ptr<taxon_t>)> & fun) { return on_prune_sig.AddAction(fun); }

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

    virtual data_ptr_t AddPairwiseDistanceDataNode(const std::string & name = "pairwise_distance") {
      auto node = AddDataNode(name);
      node->AddPullSet([this](){
        return GetPairwiseDistances();
      });
      return node;
    }

    virtual data_ptr_t AddPhylogeneticDiversityDataNode(const std::string & name = "phylogenetic_diversity") {
      auto node = AddDataNode(name);
      node->AddPull([this](){
        return GetPhylogeneticDiversity();
      });
      return node;
    }


    virtual data_ptr_t
    AddDeleteriousStepDataNode(const std::string & name = "deleterious_steps") {
      return AddDeleteriousStepDataNodeImpl(1, name);
    }

    data_ptr_t AddDeleteriousStepDataNodeImpl(bool decoy, const std::string & name = "deleterious_steps") {
      emp_assert(false, "Calculating deleterious steps requires suitable DATA_STRUCT");
      return AddDataNode(name);
    }

    template <typename T=int>
    data_ptr_t
    AddDeleteriousStepDataNodeImpl(typename std::enable_if<DATA_STRUCT::has_fitness_t::value, T>::type decoy, const std::string & name = "deleterious_steps") {
      auto node = AddDataNode(name);
      node->AddPullSet([this](){
        emp::vector<double> result;
        for (auto tax : active_taxa) {
          result.push_back(CountDeleteriousSteps(tax));
        }
        return result;
      });

      return node;
    }

    virtual data_ptr_t
    AddVolatilityDataNode(const std::string & name = "volatility") {
      return AddVolatilityDataNodeImpl(1, name);
    }

    data_ptr_t AddVolatilityDataNodeImpl(bool decoy, const std::string & name = "volatility") {
      emp_assert(false, "Calculating taxon volatility requires suitable DATA_STRUCT");
      return AddDataNode(name);
    }

    template <typename T=int>
    data_ptr_t
    AddVolatilityDataNodeImpl(typename std::enable_if<DATA_STRUCT::has_phen_t::value, T>::type decoy, const std::string & name = "volatility") {
      auto node = AddDataNode(name);
      node->AddPullSet([this](){
        emp::vector<double> result;
        for (auto tax : active_taxa) {
          result.push_back(CountPhenotypeChanges(tax));
        }
        return result;
      });

      return node;
    }

    virtual data_ptr_t
    AddUniqueTaxaDataNode(const std::string & name = "unique_taxa") {
      return AddUniqueTaxaDataNodeImpl(1, name);
    }

    data_ptr_t AddUniqueTaxaDataNodeImpl(bool decoy, const std::string & name = "unique_taxa") {
      emp_assert(false, "Calculating unique taxa requires suitable DATA_STRUCT");
      return AddDataNode(name);
    }

    template <typename T=int>
    data_ptr_t
    AddUniqueTaxaDataNodeImpl(typename std::enable_if<DATA_STRUCT::has_phen_t::value, T>::type decoy, const std::string & name = "unique_taxa") {
      auto node = AddDataNode(name);
      node->AddPullSet([this](){
        emp::vector<double> result;
        for (auto tax : active_taxa) {
          result.push_back(CountUniquePhenotypes(tax));
        }
        return result;
      });

      return node;
    }

    virtual data_ptr_t
    AddMutationCountDataNode(const std::string & name = "mutation_count", const std::string & mutation = "substitution") {
      return AddMutationCountDataNodeImpl(1, name, mutation);
    }

    data_ptr_t AddMutationCountDataNodeImpl(bool decoy, const std::string & name = "mutation_count", const std::string & mutation = "substitution") {
      emp_assert(false, "Calculating mutation count requires suitable DATA_STRUCT");
      return AddDataNode(name);
    }

    template <typename T=int>
    data_ptr_t
    AddMutationCountDataNodeImpl(typename std::enable_if<DATA_STRUCT::has_mutations_t::value, T>::type decoy, const std::string & name = "mutation_count", const std::string & mutation = "substitution") {
      auto node = AddDataNode(name);
      node->AddPullSet([this,mutation](){
        emp::vector<double> result;
        for (auto tax : active_taxa) {
          result.push_back(CountMuts(tax, mutation));
        }
        return result;
      });

      return node;
    }

    /// Add a new snapshot function.
    /// When a snapshot of the systematics is taken, in addition to the default
    /// set of functions, all user-added snapshot functions are run. Functions
    /// take a reference to a taxon as input and return the string to be dumped
    /// in the file at the given key.
    void AddSnapshotFun(const std::function<std::string(const taxon_t &)> & fun,
                        const std::string & key, const std::string & desc="") {
      user_snapshot_funs.emplace_back(fun, key, desc);
    }

    bool IsTaxonAt(int id) {
      emp_assert(id < (int) taxon_locations.size(), "Invalid taxon location", id, taxon_locations.size());
      return taxon_locations[id];
    }

    Ptr<taxon_t> GetTaxonAt(int id) {
      emp_assert(id < (int) taxon_locations.size(), "Invalid taxon location", id, taxon_locations.size());
      emp_assert(taxon_locations[id], "No taxon at specified location");
      return taxon_locations[id];
    }
    Ptr<taxon_t> GetNextTaxonAt(int id) {
      emp_assert(id < (int)next_taxon_locations.size(), "Invalid taxon location");
      emp_assert(next_taxon_locations[id], "No taxon at specified location");

      return next_taxon_locations[id];
    }

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
      return ancestor_taxa.size() + active_taxa.size() - 1;
    }

    /** This is a metric of how distinct @param tax is from the rest of the population.
     *
     * (From Vane-Wright et al., 1991; reviewed in Winter et al., 2013)
    */
    double GetTaxonDistinctiveness(Ptr<taxon_t> tax) const {return 1.0/GetDistanceToRoot(tax);}

    /** This metric (from Isaac, 2007; reviewed in Winter et al., 2013) measures how
     * distinct @param tax is from the rest of the population, weighted for the amount of
     * unique evolutionary history that it represents.
     *
     * To quantify length of evolutionary history, this method needs @param time: the current
     * time, in whatever units time is being measured in when taxa are added to the systematics
     * manager. Note that passing a time in the past will produce inacurate results (since we
     * don't know what the state of the tree was at that time).
     *
     * Assumes the tree is all connected. Will return -1 if this assumption isn't met.
    */
    double GetEvolutionaryDistinctiveness(Ptr<taxon_t> tax, double time) const {

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

    /** Calculates mean pairwise distance between extant taxa (Webb and Losos, 2000).
     * This measurement is also called Average Taxonomic Diversity (Warwick and Clark, 1998)
     * (for demonstration of equivalence see Tucker et al, 2016). This measurement tells
     * you about the amount of distinctness in the community as a whole.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred).
     *
     * This measurement assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     * */
    double GetMeanPairwiseDistance(bool branch_only=false) const {
      emp::vector<double> dists = GetPairwiseDistances(branch_only);
      return (double)Sum(dists)/dists.size();
    }

    /** Calculates summed pairwise distance between extant taxa. Tucker et al 2017 points
     *  out that this is a measure of phylogenetic richness.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred).
     *
     * This measurement assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     * */
    double GetSumPairwiseDistance(bool branch_only=false) const {
      emp::vector<double> v = GetPairwiseDistances(branch_only);
      return Sum(v);
    }

    /** Calculates variance of pairwise distance between extant taxa. Tucker et al 2017 points
     *  out that this is a measure of phylogenetic regularity.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred).
     *
     * This measurement assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     * */
    double GetVariancePairwiseDistance(bool branch_only=false) const {
      emp::vector<double> v = GetPairwiseDistances(branch_only);
      return Variance(v);
    }

    /** Calculates a vector of all pairwise distances between extant taxa.
     *
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (potentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred).
     *
     * This method assumes that the tree is fully connected. Will return -1
     * if this is not the case.
     * */
    emp::vector<double> GetPairwiseDistances(bool branch_only=false) const {
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


    /**
     * Returns a vector containing all taxa from @param time_point that were
     *
     * */
    std::set<Ptr<taxon_t>> GetCanopyExtantRoots(int time_point = 0) const {
      // NOTE: This could be made faster by doing something similar to the pairwise distance
      // function

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



    /** Counts the total number of ancestors between @param tax and MRCA, if there is one. If
     *  there is no common ancestor, distance to the root of this tree is calculated instead.
    */
    int GetDistanceToRoot(Ptr<taxon_t> tax) const {
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

    /** Counts the number of branching points leading to multiple extant taxa
     * between @param tax and the most-recent common ancestor (or the root of its subtree,
     * if no MRCA exists). This is useful because a lot
     * of stats for phylogenies are designed for phylogenies reconstructed from extant taxa.
     * These phylogenies generally only contain branching points, rather than every ancestor
     * along the way to the current taxon.*/
    int GetBranchesToRoot(Ptr<taxon_t> tax) const {
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

    /** Calculate Sackin Index of this tree (Sackin, 1972; reviewed in Shao, 1990).
     * Measures tree balance
    */
    int SackinIndex() const {
      int sackin = 0;

      for (auto taxon : active_taxa) {
        sackin += GetBranchesToRoot(taxon) + 1; // Sackin index counts root as branch
      }

      return sackin;
    }


    // Graph ToGraph() const {

    //   std::map<Ptr<taxon_t>, int> ids;
    //   int next_id = 0;

    //   for (Ptr<taxon_t> tax : active_taxa) {
    //     ids[tax] = next_id;
    //     next_id++;
    //   }

    //   for (Ptr<taxon_t> tax : ancestor_taxa) {
    //     ids[tax] = next_id;
    //     next_id++;
    //   }

    //   for (Ptr<taxon_t> tax : outside_taxa) {
    //     ids[tax] = next_id;
    //     next_id++;
    //   }

    //   Graph g(next_id);

    //   for (Ptr<taxon_t> tax : active_taxa) {
    //     if (tax->GetParent()) {
    //       g.AddEdge(ids[tax->GetParent()], ids[tax]);
    //     }
    //   }

    //   for (Ptr<taxon_t> tax : ancestor_taxa) {
    //     if (tax->GetParent()) {
    //       g.AddEdge(ids[tax->GetParent()], ids[tax]);
    //     }
    //   }

    //   for (Ptr<taxon_t> tax : outside_taxa) {
    //     if (tax->GetParent()) {
    //       g.AddEdge(ids[tax->GetParent()], ids[tax]);
    //     }
    //   }

    //   return g;
    // }

    // Graph ToMinimalGraph() const {
    //   std::map<Ptr<taxon_t>, int> ids;
    //   int next_id = 0;

    //   for (Ptr<taxon_t> tax : active_taxa) {
    //     if (tax->GetNumOff() == 1) {
    //       continue;
    //     }
    //     ids[tax] = next_id;
    //     next_id++;
    //   }

    //   for (Ptr<taxon_t> tax : ancestor_taxa) {
    //     if (tax->GetNumOff() == 1) {
    //       continue;
    //     }
    //     ids[tax] = next_id;
    //     next_id++;
    //   }

    //   for (Ptr<taxon_t> tax : outside_taxa) {
    //     if (tax->GetNumOff() == 1) {
    //       continue;
    //     }
    //     ids[tax] = next_id;
    //     next_id++;
    //   }

    //   Graph g(next_id);

    //   for (Ptr<taxon_t> tax : active_taxa) {
    //     if (tax->GetNumOff() == 1) {
    //       continue;
    //     }

    //     Ptr<taxon_t> parent = tax->GetParent();
    //     while (parent) {
    //       if (parent->GetNumOff() == 1) {
    //         parent = parent->GetParent();
    //       } else {
    //         g.AddEdge(ids[parent], ids[tax]);
    //       }
    //     }
    //   }

    //   for (Ptr<taxon_t> tax : ancestor_taxa) {
    //     if (tax->GetNumOff() == 1) {
    //       continue;
    //     }

    //     Ptr<taxon_t> parent = tax->GetParent();
    //     while (parent) {
    //       if (parent->GetNumOff() == 1) {
    //         parent = parent->GetParent();
    //       } else {
    //         g.AddEdge(ids[parent], ids[tax]);
    //       }
    //     }
    //   }

    //   for (Ptr<taxon_t> tax : outside_taxa) {
    //     if (tax->GetNumOff() == 1) {
    //       continue;
    //     }

    //     Ptr<taxon_t> parent = tax->GetParent();
    //     while (parent) {
    //       if (parent->GetNumOff() == 1) {
    //         parent = parent->GetParent();
    //       } else {
    //         g.AddEdge(ids[parent], ids[tax]);
    //       }
    //     }
    //   }

    //   return g;
    // }

    struct CollessStruct {
      double total = 0;
      emp::vector<double> ns;
    };

    CollessStruct RecursiveCollessStep(Ptr<taxon_t> curr) const {
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

    /** Calculate Colless Index of this tree (Colless, 1982; reviewed in Shao, 1990).
     * Measures tree balance. The standard Colless index only works for bifurcating trees,
     * so this will be a Colless-like Index, as suggested in
     * "Sound Colless-like balance indices for multifurcating trees" (Mir, 2018, PLoS One)
    */
    double CollessLikeIndex() const {
      GetMRCA();

      return RecursiveCollessStep(mrca).total;
    }



    void RemoveBefore(int ud) {

      // @ELD: This would be such a nice way to do it
      // but we can't because we need to notify offspring
      // when their parents are un-tracked
      // std::set<Ptr<taxon_t>> to_remove;
      // for (Ptr<taxon_t> tax : ancestor_taxa) {
      //   if (tax->GetDestructionTime() < ud) {
      //     to_remove.insert(tax);
      //   }
      // }

      // for (Ptr<taxon_t> tax : to_remove) {
      //     ancestor_taxa.erase(tax);
      //     tax.Delete();
      // }

      std::map<Ptr<taxon_t>, std::set<Ptr<taxon_t>>> to_remove;

      for (Ptr<taxon_t> tax : active_taxa) {
        Ptr<taxon_t> curr = tax;

        while (curr && !CanRemove(curr->GetParent(), ud)) {
          curr = curr->GetParent();
        }

        if (curr) {
          Ptr<taxon_t> next = curr->GetParent();
          while (next) {
            to_remove[next].insert(curr);
            curr = next;
            next = next->GetParent();
          }
        }
      }
      // std::cout << "About to remove " << to_remove.size() << " orgs" << std::endl;
      for (std::pair<Ptr<taxon_t>, std::set<Ptr<taxon_t>>> el : to_remove) {
        emp_assert(el.first->GetDestructionTime() < ud, el.first->GetDestructionTime(), ud);
        if (el.first->GetNumOff() == el.second.size()) {
          // Everything is account for
          for (auto tax : el.second) {
            tax->NullifyParent();
          }
          ancestor_taxa.erase(el.first);
          el.first.Delete();
        }
      }

    }

    bool CanRemove(Ptr<taxon_t> t, int ud) {
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

    /// Request a pointer to the Most-Recent Common Ancestor for the population.
    Ptr<taxon_t> GetMRCA() const;

    /// Request the depth of the Most-Recent Common Ancestor; return -1 for none.
    int GetMRCADepth() const;

    /// Add information about a new organism, including its stored info and parent's taxon;
    /// If you would like the systematics manager to track taxon age, you can also supply
    /// the update at which the taxon is being added.
    /// return a pointer for the associated taxon.
    void AddOrg(ORG && org, WorldPosition pos, int update=-1);
    Ptr<taxon_t> AddOrg(ORG && org, WorldPosition pos, Ptr<taxon_t> parent=nullptr, int update=-1);
    Ptr<taxon_t> AddOrg(ORG && org, Ptr<taxon_t> parent=nullptr, int update=-1);

    void AddOrg(ORG & org, WorldPosition pos, int update=-1);
    Ptr<taxon_t> AddOrg(ORG & org, WorldPosition pos, Ptr<taxon_t> parent=nullptr, int update=-1);
    Ptr<taxon_t> AddOrg(ORG & org, Ptr<taxon_t> parent=nullptr, int update=-1);


    /// Remove an instance of an organism; track when it's gone.
    bool RemoveOrg(WorldPosition pos, int time=-1);
    bool RemoveOrg(Ptr<taxon_t> taxon, int time=-1);

    void RemoveOrgAfterRepro(WorldPosition pos, int time=-1);
    void RemoveOrgAfterRepro(Ptr<taxon_t> taxon, int time=-1);

    /// Remove org from next population (for use with synchronous generations)
    // bool RemoveNextOrg(WorldPosition pos, int time=-1);
    // bool RemoveNextOrg(Ptr<taxon_t> taxon, int time=-1);

    /// Climb up a lineage...
    Ptr<taxon_t> Parent(Ptr<taxon_t> taxon) const;

    /// Print details about the Systematics manager.
    void PrintStatus(std::ostream & os=std::cout) const;

    /// Print whole lineage.
    void PrintLineage(Ptr<taxon_t> taxon, std::ostream & os=std::cout) const;

    void Snapshot(const std::string & file_path) const;

    /// Calculate the genetic diversity of the population.
    double CalcDiversity() const;

  };

  // =============================================================
  // ===                                                       ===
  // ===  Out-of-class member function definitions from above  ===
  // ===                                                       ===
  // =============================================================

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
    else if (taxon == mrca && taxon->GetNumOff() == 1) mrca = nullptr;
  }

  // Mark a taxon extinct if there are no more living members.  There may be descendants.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::MarkExtinct(Ptr<taxon_t> taxon, int time) {
    emp_assert(taxon);
    emp_assert(taxon->GetNumOrgs() == 0);

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
    // std::cout << "About to set destruction time " << time << std::endl;
    // Only need to track destruction time if we're archiving taxa
    taxon->SetDestructionTime(time);

    if (store_ancestors) {
      ancestor_taxa.insert(taxon);  // Move taxon to ancestors...
    }
    if (taxon->GetNumOff() == 0) Prune(taxon);         // ...and prune from there if needed.
  }


  // Request a pointer to the Most-Recent Common Ancestor for the population.
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

  // Request the depth of the Most-Recent Common Ancestor; return -1 for none.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  int Systematics<ORG, ORG_INFO, DATA_STRUCT>::GetMRCADepth() const {
    GetMRCA();
    if (mrca) return (int) mrca->GetDepth();
    return -1;
  }



  // Add information about a new organism, including its stored info and parent's taxon;
  // Can't return a pointer for the associated taxon because of obnoxious inheritance problems
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  // Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, WorldPosition pos, int update) {
    emp_assert(store_position, "Trying to pass position to a systematics manager that can't use it");
    // emp_assert(next_parent, "Adding organism with no parent specified and no next_parent set");
    AddOrg(org, pos, next_parent, update);
    next_parent = nullptr;
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // Can't return a pointer for the associated taxon because of obnoxious inheritance problems
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  // Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, WorldPosition pos, int update) {
    emp_assert(store_position, "Trying to pass position to a systematics manager that can't use it");
    // emp_assert(next_parent, "Adding organism with no parent specified and no next_parent set");
    AddOrg(org, pos, next_parent, update);
    next_parent = nullptr;
  }


  // Version for if you aren't tracking positions
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, Ptr<taxon_t> parent, int update) {
    return AddOrg(org, -1, parent, update);
  }

  // Version for if you aren't tracking positions
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, Ptr<taxon_t> parent, int update) {
    emp_assert(!store_position &&
              "Trying to add org to position-tracking systematics manager without position. Either specify a valid position or turn of position tracking for systematic manager.", store_position);
    return AddOrg(org, WorldPosition::invalid_id, parent, update);
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // return a pointer for the associated taxon.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG && org, WorldPosition pos, Ptr<taxon_t> parent, int update) {
    return AddOrg(org, pos, parent, update);
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // return a pointer for the associated taxon.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t>
  Systematics<ORG, ORG_INFO, DATA_STRUCT>::AddOrg(ORG & org, WorldPosition pos, Ptr<taxon_t> parent, int update) {
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
      on_new_sig.Trigger(cur_taxon, org);
      if (store_active) active_taxa.insert(cur_taxon);       // Store new taxon.
      if (parent) parent->AddOffspring(cur_taxon);                    // Track tree info.

      cur_taxon->SetOriginationTime(update);
    }
    // std::cout << "about to store poisition" << std::endl;
    if (store_position && pos.GetIndex() >= 0) {
      if (pos.GetPopID()) {
        if (pos.GetIndex() >= next_taxon_locations.size()) {
          next_taxon_locations.resize(pos.GetIndex()+1);
        }
        next_taxon_locations[pos.GetIndex()] = cur_taxon;

      } else {
        if (pos.GetIndex() >= taxon_locations.size()) {
          taxon_locations.resize(pos.GetIndex()+1);
        }
        taxon_locations[pos.GetIndex()] = cur_taxon;
      }
    }

    cur_taxon->AddOrg();                    // Record the current organism in its taxon.
    total_depth += cur_taxon->GetDepth();   // Track the total depth (for averaging)

    if (to_be_removed) {
      RemoveOrg(to_be_removed, removal_time);
      to_be_removed = nullptr;
    }

    most_recent = cur_taxon;
    return cur_taxon;                       // Return the taxon used.
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrgAfterRepro(WorldPosition pos, int time) {
    emp_assert(store_position, "Trying to remove org based on position from systematics manager that doesn't track it.");

    if (pos.GetIndex() >= taxon_locations.size() || !taxon_locations[pos.GetIndex()]) {
      // There's not actually a taxon here
      return;
    }

    RemoveOrgAfterRepro(taxon_locations[pos.GetIndex()], time);
    removal_pos = pos.GetIndex();
  }

  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrgAfterRepro(Ptr<taxon_t> taxon, int time) {
    if (to_be_removed != nullptr) {
      RemoveOrg(to_be_removed, removal_time);
      taxon_locations[removal_pos] = nullptr;
      to_be_removed = nullptr;
      removal_pos = -1;
    }
    to_be_removed = taxon;
    // std::cout << "Setting remove time to " << time << std::endl;
    removal_time = time;
  }


  // Remove an instance of an organism; track when it's gone.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  bool Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrg(WorldPosition pos, int time) {
    emp_assert(store_position, "Trying to remove org based on position from systematics manager that doesn't track it.");

    if (pos.GetPopID() == 0) {
      emp_assert(pos.GetIndex() < taxon_locations.size(), "Invalid position requested for removal", pos.GetIndex(), taxon_locations.size());
      bool active = false;
      if (taxon_locations[pos.GetIndex()]) {
        //TODO: Figure out how this can ever not be true
        active = RemoveOrg(taxon_locations[pos.GetIndex()], time);
      }
      taxon_locations[pos.GetIndex()] = nullptr;
      return active;
    } else {
      emp_assert(pos.GetIndex() < next_taxon_locations.size(), "Invalid position requested for removal", pos.GetIndex(), taxon_locations.size());
      bool active = RemoveOrg(next_taxon_locations[pos.GetIndex()], time);
      next_taxon_locations[pos.GetIndex()] = nullptr;
      return active;
    }
  }

  // Remove an instance of an organism; track when it's gone.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  bool Systematics<ORG, ORG_INFO, DATA_STRUCT>::RemoveOrg(Ptr<taxon_t> taxon, int time) {
    emp_assert(taxon);

    // Update stats
    org_count--;
    total_depth -= taxon->GetDepth();

    // emp_assert(Has(active_taxa, taxon));
    const bool active = taxon->RemoveOrg();
    if (!active) MarkExtinct(taxon, time);

    return active;
  }

  // Climb up a lineage...
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  Ptr<typename Systematics<ORG, ORG_INFO, DATA_STRUCT>::taxon_t> Systematics<ORG, ORG_INFO, DATA_STRUCT>::Parent(Ptr<taxon_t> taxon) const {
    emp_assert(taxon);
    emp_assert(Has(active_taxa, taxon));
    return taxon->GetParent();
  }

  // Print details about the Systematics manager.
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

  // Print whole lineage.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  void Systematics<ORG, ORG_INFO, DATA_STRUCT>::PrintLineage(Ptr<taxon_t> taxon, std::ostream & os) const {
    os << "Lineage:\n";
    while (taxon) {
      os << taxon->GetInfo() << std::endl;
      taxon = taxon->GetParent();
    }
  }

  /// Take a snapshot of current state of taxon phylogeny.
  /// WARNING: Current, this function assumes one parent taxon per-taxon.
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
      if (cur_taxon->GetParent() == nullptr) { return "[NONE]"; }
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

    // Update file w/active taxa information
    for (auto tax : active_taxa) {
      cur_taxon = tax;
      file.Update();
    }

    // Update file w/ancestor taxa information
    for (auto tax : ancestor_taxa) {
      cur_taxon = tax;
      file.Update();
    }

    // Update file w/outside taxa information
    for (auto tax : outside_taxa) {
      cur_taxon = tax;
      file.Update();
    }

  }

  // Calculate the genetic diversity of the population.
  template <typename ORG, typename ORG_INFO, typename DATA_STRUCT>
  double Systematics<ORG, ORG_INFO, DATA_STRUCT>::CalcDiversity() const {
    return emp::Entropy(active_taxa, [](Ptr<taxon_t> x){ return x->GetNumOrgs(); }, (double) org_count);
  }
}

#endif
