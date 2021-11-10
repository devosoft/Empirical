/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file OEE.hpp
 *  @brief TODO.
 */

#ifndef EMP_EVOLVE_OEE_HPP_INCLUDE
#define EMP_EVOLVE_OEE_HPP_INCLUDE

#include <deque>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/BloomFilter.hpp"
#include "../datastructs/set_utils.hpp"
#include "../datastructs/vector_utils.hpp"

#include "Systematics.hpp"

namespace emp {

  // Setup possible types for keeping track of what we've seen for novelty

  template <typename SKEL_TYPE>
  class SeenSet {
    std::set<SKEL_TYPE> s;
    public:
    using skel_t = SKEL_TYPE;
    // Placeholders to ensure that constructor signature is same as bloom filter
    SeenSet(int placeholder_1 = 200000, double placeholder_2 = 0.0001) { ; }
    void insert(const skel_t & val) {s.insert(val);}
    bool contains(const skel_t & val) {return Has(s, val);}
  };

  class SeenBloomFilter {
    BloomFilter b;

    public:

    using skel_t = std::string;
    SeenBloomFilter(int bloom_count = 200000, double false_positive = 0.0001) {
      BloomParameters parameters;

      // How many elements roughly do we expect to insert?
      parameters.projected_element_count = bloom_count;

      // Maximum tolerable false positive probability? (0,1)
      parameters.false_positive_probability = false_positive;

      if (!parameters)
      {
        std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
      }

      parameters.compute_optimal_parameters();
      b = BloomFilter(parameters);
    }

    void insert(const skel_t & val) {b.insert(val);}
    bool contains(const skel_t & val) {return b.contains(val);}
  };


  template <typename SYSTEMATICS_TYPE, typename SKEL_TYPE = typename SYSTEMATICS_TYPE::info_t, typename SEEN_TYPE = SeenSet<SKEL_TYPE>>
  class OEETracker {
    private:
    using systematics_t = SYSTEMATICS_TYPE;
    using taxon_t = typename systematics_t::taxon_t;
    using info_t = typename systematics_t::info_t;
    using hash_t = typename Ptr<taxon_t>::hash_t;
    using fun_calc_complexity_t = std::function<double(const SKEL_TYPE&)>;
    using fun_calc_data_t = std::function<SKEL_TYPE(info_t &)>; // TODO: Allow other skeleton types

    struct snapshot_info_t {
      Ptr<taxon_t> taxon = nullptr; // This is what the systematics manager has
      Ptr<SKEL_TYPE> skel = nullptr;
      int count = 0; // Count of this taxon at time of snapshot

      ~snapshot_info_t() {if (skel){skel.Delete();}}
      // bool operator==(const snapshot_info_t & other) const {return other.taxon == taxon;}
    };

    std::deque<emp::vector<snapshot_info_t>> snapshots;
    std::deque<int> snapshot_times;
    Ptr<systematics_t> systematics_manager;

    std::map<SKEL_TYPE, int> prev_coal_set;
    // std::unordered_set<SKEL_TYPE> seen;

    fun_calc_data_t skeleton_fun;
    fun_calc_complexity_t complexity_fun;
    int generation_interval = 10;
    int resolution = 10;

    DataManager<double, data::Current, data::Info> data_nodes;
    SEEN_TYPE seen;
    bool prune_top;

    public:
    OEETracker(Ptr<systematics_t> s, fun_calc_data_t d, fun_calc_complexity_t c, bool remove_top = false, int bloom_count = 200000, double bloom_false_positive = .0001) :
      systematics_manager(s), skeleton_fun(d), complexity_fun(c), seen(bloom_count, bloom_false_positive), prune_top(remove_top) {

      emp_assert(s->GetStoreAncestors(), "OEE tracker only works with systematics manager where store_ancestor is set to true");

      data_nodes.New("change");
      data_nodes.New("novelty");
      data_nodes.New("diversity");
      data_nodes.New("complexity");

    }

    ~OEETracker() {}

    int GetResolution() const {return resolution;}
    int GetGenerationInterval() const {return generation_interval;}

    void SetResolution(int r) {resolution = r;}
    void SetGenerationInterval(int g) {generation_interval = g;}

    void Update(size_t gen, int ud = -1) {
      if (Mod((int)gen, resolution) == 0) {
        if (ud == -1) {
          ud = gen;
        }
        auto & sys_active = systematics_manager->GetActive();

        snapshots.emplace_back(sys_active.size());
        int i = 0;
        for (auto tax : sys_active) {
          snapshots.back()[i].taxon = tax;
          info_t info = tax->GetInfo();
          snapshots.back()[i].skel.New(skeleton_fun(info));
          snapshots.back()[i].count = tax->GetNumOrgs();
          i++;
        }

        snapshot_times.push_back(ud);
        if ((int)snapshots.size() > generation_interval/resolution + 1) {
          if (prune_top) {
            systematics_manager->RemoveBefore(snapshot_times.front() - 1);
          }
          snapshot_times.pop_front();

          snapshots.pop_front();
        }
        CalcStats(ud);
      }
    }

    void CalcStats(size_t ud) {
      std::map<SKEL_TYPE, int> coal_set = CoalescenceFilter(ud);
      int change = 0;
      int novelty = 0;
      double most_complex = 0;
      double diversity = 0;
      if (coal_set.size() > 0) {
        diversity = Entropy(coal_set, [](std::pair<SKEL_TYPE, int> entry){return entry.second;});
      }

      for (auto & tax : coal_set) {
        if (!Has(prev_coal_set, tax.first)) {
          change++;
        }
        if (!seen.contains(tax.first)) {
          novelty++;
          seen.insert(tax.first);
        }
        double complexity = complexity_fun(tax.first);
        if (complexity > most_complex) {
          most_complex = complexity;
        }
      }

      data_nodes.Get("change").Add(change);
      data_nodes.Get("novelty").Add(novelty);
      data_nodes.Get("diversity").Add(diversity);
      data_nodes.Get("complexity").Add(most_complex);

      std::swap(prev_coal_set, coal_set);
    }

    std::map<SKEL_TYPE, int> CoalescenceFilter(size_t ud) {

      emp_assert(emp::Mod(generation_interval, resolution) == 0, "Generation interval must be a multiple of resolution", generation_interval, resolution);

      std::map<SKEL_TYPE, int> res;

      if ((int)snapshots.size() <= generation_interval/resolution) {
        return res;
      }

      std::set<Ptr<taxon_t>> extant_canopy_roots = systematics_manager->GetCanopyExtantRoots(snapshot_times.front());
      for ( snapshot_info_t & t : snapshots.front()) {
        if (Has(extant_canopy_roots, t.taxon)) {
          if (Has(res, *(t.skel))) {
            res[*(t.skel)] += t.count;
          } else {
            res[*(t.skel)] = t.count;
          }
        }
      }

      return res;
    }


    Ptr<DataNode<double, data::Current, data::Info>> GetDataNode(const std::string & name) {
      return &(data_nodes.Get(name));
    }

  };

  // Helper function for skeletonization when organism is a sequence of

  // Assumes org is sequence of inst_type
  template <typename ORG_TYPE, typename INST_TYPE>
  emp::vector<INST_TYPE> Skeletonize(ORG_TYPE & org, const INST_TYPE null_value, std::function<double(ORG_TYPE&)> fit_fun) {
    emp_assert(org.size() > 0, "Empty org passed to skeletonize");

    emp::vector<INST_TYPE> skeleton;
    // Some fitness functions may require the org to be const and smoe may require it to not be
    // We can let the compiler deducce whetehr ORG_TYPE is const or not.
    // But the test org really needs to not be const
    typename std::remove_const<ORG_TYPE>::type test_org = ORG_TYPE(org);
    double fitness = fit_fun(test_org);

    for (int i = 0; i < (int)org.size(); i++) {
      test_org[i] = null_value;
      double new_fitness = fit_fun(test_org);
      if (new_fitness < fitness) {
        skeleton.push_back(org[i]);
      }
      test_org[i] = org[i];
    }

    return skeleton;
  }


}

#endif // #ifndef EMP_EVOLVE_OEE_HPP_INCLUDE
