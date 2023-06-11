/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file World_structure.hpp
 *  @brief Functions for popular world structure methods.
 */

#ifndef EMP_EVOLVE_WORLD_STRUCTURE_HPP_INCLUDE
#define EMP_EVOLVE_WORLD_STRUCTURE_HPP_INCLUDE

#include <set>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../data/Trait.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"

namespace emp {

  template <typename ORG> class World;

  /// A class to track positions in World.
  /// For the moment, the only informaiton beyond index is active (vs. next) population when
  /// using synchronous generations.
  //
  //  Developer NOTE: For efficiency, internal class members are uint32_t, but to prevent compiler
  //                  warnings, size_t values are accepted; asserts ensure safe conversions.
  class WorldPosition {
  private:
    uint32_t index;   ///<  Position of this organism in the population.
    uint32_t pop_id;  ///<  ID of the population we are in; 0 is always the active population.

  public:
    static constexpr size_t invalid_id = (uint32_t) -1;

    WorldPosition() : index(invalid_id), pop_id(invalid_id) { ; }
    WorldPosition(size_t _id, size_t _pop_id=0) : index((uint32_t) _id), pop_id((uint32_t) _pop_id) {
      emp_assert(_id <= invalid_id);
      emp_assert(_pop_id <= invalid_id);
    }
    WorldPosition(const WorldPosition &) = default;

    uint32_t GetIndex() const { return index; }
    uint32_t GetPopID() const { return pop_id; }

    bool IsActive() const { return pop_id == 0; }
    bool IsValid() const { return index != invalid_id; }

    WorldPosition & SetActive(bool _active=true) { pop_id = 0; return *this; }
    WorldPosition & SetPopID(size_t _id) { emp_assert(_id <= invalid_id); pop_id = (uint32_t) _id; return *this; }
    WorldPosition & SetIndex(size_t _id) { emp_assert(_id <= invalid_id); index = (uint32_t) _id; return *this; }
    WorldPosition & MarkInvalid() { index = invalid_id; pop_id = invalid_id; return *this; }
  };

  /// A vector that can be indexed with a WorldPosition
  template <typename T>
  class WorldVector : public emp::array<emp::vector<T>, 2> {
  public:
    using base_t = emp::array<emp::vector<T>, 2>;

    /// Test if a position is currently valid.
    bool IsValid(WorldPosition pos) const {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      emp_assert(pop_id < 2);
      return id < base_t::operator[](pop_id).size();
    }

    /// Make sure position is valid; if not expand relevant vector.
    void MakeValid(WorldPosition pos) {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      emp_assert(pop_id < 2);
      if (id >= base_t::operator[](pop_id).size()) {
        base_t::operator[](pop_id).resize(id+1);
      }
    }

    T & operator()(WorldPosition pos) {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      return base_t::operator[](pop_id)[id];
    }
    const T & operator()(WorldPosition pos) const {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      return base_t::operator[](pop_id)[id];
    }
  };

  /// Set the population to be a set of pools that are individually well mixed, but with limited
  /// migtation.  Arguments are the number of pools, the size of each pool, and whether the
  /// generations should be synchronous (true) or not (false, default).
  template <typename ORG>
  void SetPools(World<ORG> & world, size_t num_pools,
                size_t pool_size, bool synchronous_gen=false) {
    world.Resize(pool_size, num_pools);
    world.MarkSynchronous(synchronous_gen);
    world.MarkSpaceStructured(true).MarkPhenoStructured(false);

    // -- Setup functions --
    // Inject in an empty pool -or- randomly if none empty
    world.SetAddInjectFun( [&world,pool_size](Ptr<ORG> new_org) {
      for (size_t id = 0; id < world.GetSize(); id += pool_size) {
        if (world.IsOccupied(id) == false) return WorldPosition(id);
      }
      return WorldPosition(world.GetRandomCellID());
    });

    // Neighbors are everyone in the same pool.
    world.SetGetNeighborFun( [&world,pool_size](WorldPosition pos) {
      const size_t pool_start = (pos.GetIndex() / pool_size) * pool_size;
      return pos.SetIndex(pool_start + world.GetRandom().GetUInt(pool_size));
    });

    world.SetKillOrgFun( [&world](){
      const size_t kill_id = world.GetRandomCellID();
      world.RemoveOrgAt(kill_id);
      return kill_id;
    });

    if (synchronous_gen) {
      // Place births in the next open spot in the new pool (or randomly if full!)
      world.SetAddBirthFun( [&world,pool_size](Ptr<ORG> new_org, WorldPosition parent_pos) {
        emp_assert(new_org);                                  // New organism must exist.
        const size_t parent_id = parent_pos.GetIndex();
        const size_t pool_id = parent_id / pool_size;
        const size_t start_id = pool_id * pool_size;
        for (size_t id = start_id; id < start_id+pool_size; id++) {
          if (world.IsOccupied(WorldPosition(id,1)) == false) {  // Search for an open position...
            return WorldPosition(id, 1);
          }
        }
        WorldPosition pos = world.GetRandomNeighborPos(parent_pos);  // Placed near parent, in next pop.
        return pos.SetPopID(1);
      });
      world.SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neighbor in current population.
      world.SetAddBirthFun( [&world](Ptr<ORG> new_org, WorldPosition parent_pos) {
        auto pos = world.GetRandomNeighborPos(parent_pos);
        return pos; // Place org in existing population.
      });
      world.SetAttribute("SynchronousGen", "False");
    }

    world.SetAttribute("PopStruct", "Pools");
    world.SetSynchronousSystematics(synchronous_gen);
  }


  /// Set the population to use a MapElites structure.  This means that organism placement has
  /// two key components:
  /// 1: Organism position is based on their phenotypic traits.
  /// 2: Organisms must have a higher fitness than the current resident of a position to steal it.
  ///
  /// Note: Since organisms compete with their predecessors for space in the populations,
  /// synchronous generations do not make sense.
  ///
  /// This for version will setup a MAP-Elites world; traits to use an how many bins for each
  /// (trait counts) must be provided.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, TraitSet<ORG> traits,
                    const emp::vector<size_t> & trait_counts) {
    world.Resize(trait_counts);  // World sizes are based on counts of traits options.
    world.MarkSynchronous(false);
    world.MarkSpaceStructured(false).MarkPhenoStructured(true);

    // -- Setup functions --
    // Inject into the appropriate positon based on phenotype.  Note that an inject will fail
    // if a more fit organism is already in place; you must run clear first if you want to
    // ensure placement.
    world.SetAddInjectFun( [&world,traits,trait_counts](Ptr<ORG> new_org) {
      // Determine tha position that this phenotype fits in.
      double org_fitness = world.CalcFitnessOrg(*new_org);
      size_t id = traits.EvalBin(*new_org, trait_counts);
      double cur_fitness = world.CalcFitnessID(id);

      if (cur_fitness > org_fitness) return WorldPosition();  // Return invalid position!
      return WorldPosition(id);
    });

    // Map-Elites does not have a concept of neighbors.
    world.SetGetNeighborFun( [](WorldPosition pos) { emp_assert(false); return pos; });

    // Map-Elites doesn't have a real meaning for killing organisms, so do so randomly.
    world.SetKillOrgFun( [&world](){
      const size_t kill_id = world.GetRandomCellID();
      world.RemoveOrgAt(kill_id);
      return kill_id;
    });

    // Birth is effectively the same as inject.
    world.SetAddBirthFun( [&world,traits,trait_counts](Ptr<ORG> new_org, WorldPosition parent_pos) {
      (void) parent_pos; // Parent position is not needed for MAP Elites.
      // Determine tha position that this phenotype fits in.
      double org_fitness = world.CalcFitnessOrg(*new_org);
      size_t id = traits.EvalBin(*new_org, trait_counts);
      double cur_fitness = world.CalcFitnessID(id);

      if (cur_fitness > org_fitness) return WorldPosition();  // Return invalid position!
      return WorldPosition(id);
    });

    world.SetAttribute("SynchronousGen", "False");
    world.SetAttribute("PopStruct", "MapElites");
    world.SetSynchronousSystematics(false);
  }

  /// Setup a MAP-Elites world, given the provided set of traits.
  /// Requires world to already have a size; that size is respected when deciding trait bins.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, TraitSet<ORG> traits) {
    emp::vector<size_t> trait_counts;
    emp_assert(traits.GetSize() > 0);

    // If there's only a single trait, it should get the full population.
    if (traits.GetSize() == 1) {
      trait_counts.push_back(world.GetSize());
      SetMapElites(world, traits, trait_counts);
      return;
    }
    const size_t num_traits = traits.GetSize();
    size_t trait_size = 1;
    while (std::pow(trait_size+1, num_traits) < world.GetSize()) trait_size++;
    trait_counts.resize(num_traits, trait_size);
    SetMapElites(world, traits, trait_counts);
  }

  /// Setup a MAP-Elites world, given the provided trait counts (number of bins).
  /// Requires world to already have a phenotypes that those counts are applied to.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, const emp::vector<size_t> & trait_counts) {
    SetMapElites(world, world.GetPhenotypes(), trait_counts);
  }

  /// Setup a MAP-Elites world, given the provided worlds already has size AND set of phenotypes.
  /// Requires world to already have a size; that size is respected when deciding trait bins.
  template <typename ORG>
  void SetMapElites(World<ORG> & world) { SetMapElites(world, world.GetPhenotypes()); }



  /// DiverseElites is similar to MAP-Elites, but rather than merely keep the elites on
  /// a pre-defined grid, it merely tries to maintain maximal distance between elites in
  /// trait space.  The main advantages to this technique are (1) It's easy to build
  /// up an inital population that grows in diversity over time, and (2) You don't need to
  /// predefine box sizes or even limits to trait values.

  /// Set the population to use a DiverseElites structure.  This means that organism placement has
  /// two key components:
  /// 1: Organism position is in continuous space based on phenotypic traits.
  /// 2: When the population is full, nearby organisms must battle to keep their position.
  ///
  /// Note: Since organisms compete with their predecessors for space in the populations,
  /// synchronous generations do not make sense.

  /// Build a class to track distances between organisms.
  // Note: Assuming that once a position is filled it will never be empty again.
  template <typename ORG>
  struct World_MinDistInfo {
    static constexpr size_t ID_NONE = (size_t) -1;  ///< ID for organism does not exist.

    emp::vector<size_t> nearest_id;  ///< For each individual, whom are they closest to?
    emp::vector<double> distance;    ///< And what is their distance?

    World<ORG> & world;              ///< World object being tracked.
    TraitSet<ORG> traits;            ///< Traits we are tryng to spread
    emp::vector<double> min_vals;    ///< Smallest value found for each trait.
    emp::vector<double> max_vals;    ///< Largest value found for each trait.
    emp::vector<double> bin_width;   ///< Largest value found for each trait.

    bool is_setup;                          ///< Have we initialized the internal data stucture?
    size_t num_trait_bins;                  ///< How many bins should we use for each trait?
    size_t num_total_bins;                  ///< How many bins are there overall?
    emp::vector<std::set<size_t>> bin_ids;  ///< Which org ids fall into each bin?
    emp::vector<size_t> org_bins;           ///< Which bin is each org currently in?

    World_MinDistInfo(World<ORG> & in_world, const TraitSet<ORG> & in_traits)
     : nearest_id(), distance(), world(in_world), traits(in_traits)
     , min_vals(traits.GetSize(), std::numeric_limits<double>::max())
     , max_vals(traits.GetSize(), std::numeric_limits<double>::min())
     , bin_width(traits.GetSize(), 0.00001)
     , is_setup(false), num_trait_bins(0), num_total_bins(0), bin_ids(), org_bins()
     { ; }

    double CalcDist(size_t id1, size_t id2) {
      emp::vector<double> offsets = traits.CalcOffsets(world.GetOrg(id1), world.GetOrg(id2));
      double dist = 0.0;
      for (double offset : offsets) dist += offset * offset;
      return dist;
    }

    // Helper function for testing an organism against everything in a specified bin.
    void Refresh_AgainstBin(size_t refresh_id, size_t target_bin) {
      emp_assert(target_bin < bin_ids.size(), target_bin, bin_ids.size());
      for (size_t id2 : bin_ids[target_bin]) {
        if (id2 == refresh_id) continue;
        const double cur_dist = CalcDist(id2, refresh_id);
        if (cur_dist < distance[refresh_id]) {
          distance[refresh_id] = cur_dist;
          nearest_id[refresh_id] = id2;
        }
        if (cur_dist < distance[id2]) {
          distance[id2] = cur_dist;
          nearest_id[id2] = refresh_id;
        }
      }
    }

    // Find the closest connection to a position again; update neighbors as well!
    void Refresh(size_t refresh_id, size_t start_id = 0) {
      emp_assert(refresh_id < world.GetSize()); // Make sure ID is legal.
      nearest_id[refresh_id] = ID_NONE;
      distance[refresh_id] = std::numeric_limits<double>::max();

      // First compare against everything else in the current bin.
      size_t bin_id = org_bins[refresh_id];
      Refresh_AgainstBin(refresh_id, bin_id);

      // Then check all neighbor bins.  Ignoring diagnols for now since they could be expensive...
      // (though technically we need them...)
      size_t trait_offset = 1;
      for (size_t trait_id = 0; trait_id < traits.GetSize(); trait_id++) {
        size_t prev_bin_id = bin_id - trait_offset;
        if (prev_bin_id < num_total_bins) {
          Refresh_AgainstBin(refresh_id, prev_bin_id);
        }
        size_t next_bin_id = bin_id + trait_offset;
        if (next_bin_id < num_total_bins) {
          Refresh_AgainstBin(refresh_id, next_bin_id);
        }
        trait_offset *= num_trait_bins;
      }

    }

    /// Calculate which bin an organism should be in.
    size_t CalcBin(size_t id) {
      static emp::vector<double> t_vals;
      t_vals = traits.EvalValues(world.GetOrg(id));
      size_t scale = 1;
      size_t bin_id = (size_t) -1;
      for (size_t i = 0; i < traits.GetSize(); i++) {
        const size_t cur_bin = (size_t) ((t_vals[i] - min_vals[i]) / bin_width[i]);
        emp_assert(cur_bin < num_total_bins);
        bin_id += cur_bin * scale;
        scale *= num_trait_bins;
      }
      emp_assert(bin_id < num_total_bins, bin_id, num_total_bins, scale);
      return bin_id;
    }

    /// Reset all of the bins in the multidimensional grid for nearest-neighbor analysis.
    void ResetBins() {
      bin_ids.resize(num_total_bins);
      for (auto & bin : bin_ids) bin.clear();
      for (size_t trait_id = 0; trait_id < traits.GetSize(); trait_id++) {
        bin_width[trait_id] = (max_vals[trait_id] - min_vals[trait_id]) / (double) num_trait_bins;
      }
      org_bins.resize(world.GetSize());
      for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
        size_t cur_bin = CalcBin(org_id);
        org_bins[org_id] = cur_bin;
        bin_ids[cur_bin].insert(org_id);
      }
    }

    void Setup() {
      const size_t num_orgs = world.GetSize();
      emp_assert(num_orgs >= 2); // Must have at least 2 orgs in the population to setup.
      const size_t num_traits = traits.GetSize();
      emp_assert(num_traits >= 1); // We must have at least one dimension!

      nearest_id.resize(num_orgs);
      distance.resize(num_orgs);

      // How many bins should each trait be divided into?
      num_trait_bins = (size_t) (std::pow(num_orgs, 1.0 / (double)num_traits) + 0.5);
      num_total_bins = (size_t) (std::pow(num_trait_bins, num_traits) + 0.5);
      ResetBins();

      // Setup (Refresh) all distances.
      for (size_t id = 0; id < num_orgs; id++) { Refresh(id, id+1); }
      is_setup = true;
    }

    void Clear() {
      nearest_id.resize(0);
      distance.resize(0);
      is_setup = false;
    }

    /// Find the best organism to kill in the popualtion.  In this case, find the two closest organisms
    /// and kill the one with the lower fitness.
    size_t FindKill() {
      if (!is_setup) Setup();  // The first time we run out of space and need to kill, setup structure!

      emp_assert(distance.size() > 0);  // After setup, we should always have distances stored.

      const size_t min_id = emp::FindMinIndex(distance);
      emp_assert(min_id >= 0 && min_id < world.GetSize(), min_id);
      emp_assert(nearest_id[min_id] >= 0 && nearest_id[min_id] < world.GetSize(),
                 min_id, distance[min_id], nearest_id[min_id], distance.size());
      if (world.CalcFitnessID(min_id) < world.CalcFitnessID(nearest_id[min_id])) return min_id;
      else return nearest_id[min_id];
    }

    /// Return an empty world position.  If none are available, return the position of an org to be killed.
    size_t GetBirthPos(size_t world_size) {
      // If there's room in the world for one more, get the next empty position.
      if (world.GetSize() < world_size) { return world.GetSize(); }
      // Otherwise, determine whom to kill return their position to be used.
      return FindKill();
    }

    /// Assume a position has changed; refresh it AND everything that had it as a closest connection.
    void Update(size_t pos) {
      /// Determine if this new point extends the range of any phenotypes.
      bool update_chart = false;
      emp::vector<double> cur_vals = traits.EvalValues(world.GetOrg(pos));
      for (size_t i = 0; i < cur_vals.size(); i++) {
        if (cur_vals[i] <= min_vals[i]) {
          min_vals[i] = cur_vals[i] - bin_width[i]/2.0;
          update_chart = true;
        }
        if (cur_vals[i] >= max_vals[i]) {
          max_vals[i] = cur_vals[i] + bin_width[i]/2.0;
          update_chart = true;
        }
      }

      // Until min-dist tracking structure is setup, don't worry about maintaining.
      if (!is_setup) return;
      emp_assert(pos < world.GetSize());

      /// Remove org if from the bin we currently have it in.
      bin_ids[org_bins[pos]].erase(pos);

      /// Determine if we need to re-place all orgs in the structure
      if (update_chart == true) {
        ResetBins();
        org_bins[pos] = CalcBin(pos);
        bin_ids[org_bins[pos]].insert(pos);

        // Rescaled bins might skew distances.  Refresh everyone!
        for (size_t id = 0; id < world.GetSize(); id++) {
          Refresh(id);
        }
        emp_assert(org_bins[pos] != (size_t) -1);
      }

      /// Otherwise just update closest connections to this org.
      else {
        org_bins[pos] = CalcBin(pos);
        bin_ids[org_bins[pos]].insert(pos);
        for (size_t id = 0; id < world.GetSize(); id++) {
          if (nearest_id[id] == pos) Refresh(id);
        }
        Refresh(pos);
        emp_assert(org_bins[pos] != (size_t) -1);
      }

      emp_assert(OK());
    }

    /// A debug function to make sure the internal state is all valid.
    bool OK() {
      // These tests only matter BEFORE Setup() is run.
      emp_assert(is_setup || nearest_id.size() == 0);
      emp_assert(is_setup || distance.size() == 0);

      // Tests for AFTER Setup() is run.

      if (is_setup) {
        const size_t num_orgs = world.GetSize();
        emp_assert(nearest_id.size() == num_orgs);
        emp_assert(distance.size() == num_orgs);
        for (size_t i = 0; i < num_orgs; i++) {
          emp_assert(org_bins[i] < num_total_bins, i, org_bins[i], num_total_bins,
                     world.GetNumOrgs());
        }
        size_t org_count = 0;
        for (size_t i = 0; i < num_total_bins; i++) {
          org_count += bin_ids[i].size();
          for (size_t org_id : bin_ids[i]) {
            (void) org_id;
            emp_assert(org_bins[org_id] == i);
          }
        }
        emp_assert(org_count == num_orgs, org_count, num_orgs, world.GetNumOrgs());
      }

      return true;
    }
  };

  /// This first version will setup a Diverse-Elites world and specify traits to use.
  template <typename ORG>
  void SetDiverseElites(World<ORG> & world, TraitSet<ORG> traits, size_t world_size) {
    world.MarkSynchronous(false);
    world.MarkSpaceStructured(false).MarkPhenoStructured(true);

    // Build a pointer to the current information (and make sure it's deleted later)
    Ptr<World_MinDistInfo<ORG>> info_ptr = NewPtr<World_MinDistInfo<ORG>>(world, traits);
    world.OnWorldDestruct([info_ptr]() mutable { info_ptr.Delete(); });

    // Make sure to update info whenever a new org is placed into the population.
    world.OnPlacement( [info_ptr](size_t pos) mutable { info_ptr->Update(pos); } );

    // -- Setup functions --
    // Inject into the appropriate positon based on phenotype.  Note that an inject will fail
    // if a more fit organism is already in place; you must run clear first if you want to
    // ensure placement.
    world.SetAddInjectFun( [&world, traits, world_size, info_ptr](Ptr<ORG> new_org) {
      size_t pos = info_ptr->GetBirthPos(world_size);
      return WorldPosition(pos);
    });

    // Diverse Elites does not have a concept of neighbors.
    // @CAO Or should we return closest individual, which we already save?
    world.SetGetNeighborFun( [](WorldPosition pos) { emp_assert(false); return pos; });

    // Find the two closest organisms and kill the lower fit one.  (Killing sparsely...)
    // Must unsetup population for next birth to work.
    world.SetKillOrgFun( [&world, info_ptr](){
      const size_t last_id = world.GetSize() - 1;
      world.Swap(info_ptr->FindKill(), last_id);
      info_ptr->is_setup = false;
      world.RemoveOrgAt(last_id);
      world.Resize(last_id);
      return last_id;
    });

    // Birth is effectively the same as inject.
    world.SetAddBirthFun( [&world, traits, world_size, info_ptr](Ptr<ORG> new_org, WorldPosition parent_pos) {
      (void) parent_pos;
      size_t pos = info_ptr->GetBirthPos(world_size);
      return WorldPosition(pos);
    });

    world.SetAttribute("SynchronousGen", "False");
    world.SetAttribute("PopStruct", "DiverseElites");
    world.SetSynchronousSystematics(false);
  }

  /// Setup a Diverse-Elites world, given the provided world already has set of phenotypes.
  template <typename ORG>
  void SetDiverseElites(World<ORG> & world, size_t world_size) {
    SetDiverseElites(world, world.GetPhenotypes(), world_size);
  }
}

#endif // #ifndef EMP_EVOLVE_WORLD_STRUCTURE_HPP_INCLUDE
