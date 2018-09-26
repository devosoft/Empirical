#ifndef EMP_OEE_STATS_H
#define EMP_OEE_STATS_H

#include "Systematics.h"
#include "bloom_filter.hpp"
#include "base/vector.h"
#include "base/Ptr.h"
#include "tools/set_utils.h"
#include "tools/vector_utils.h"

#include <deque>

namespace emp {

    namespace datastruct {
        template <typename SKEL_TYPE>
        struct oee_data : public no_data {

            Ptr<SKEL_TYPE> skeleton = nullptr;
            ~oee_data(){if(skeleton){skeleton.Delete();}}
        };
    };

    template <typename ORG, typename ORG_INFO, typename SKEL_TYPE, typename DATA_STRUCT=emp::datastruct::oee_data<SKEL_TYPE>>
    class OEETracker {
        private:
        using taxon_t = Taxon<ORG_INFO, DATA_STRUCT>;
        using hash_t = typename Ptr<taxon_t>::hash_t;
        using fun_calc_complexity_t = std::function<double(const SKEL_TYPE&)>;
        using fun_calc_data_t = std::function<SKEL_TYPE(const ORG_INFO&)>; // TODO: Allow other skeleton types

        struct snapshot_info_t {
            Ptr<taxon_t> taxon = nullptr; // This is what the systematics manager has
            int count; // Count of this taxon at time of snapshot
            // bool operator==(const snapshot_info_t & other) const {return other.taxon == taxon;}
        };

        std::deque<emp::vector<snapshot_info_t>> snapshots;
        std::deque<int> snapshot_times;
        Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> systematics_manager;

        std::map<SKEL_TYPE, int> prev_coal_set;
        // std::unordered_set<SKEL_TYPE> seen;

        fun_calc_complexity_t complexity_fun;
        fun_calc_data_t skeleton_fun;
        int generation_interval = 10;
        int resolution = 10;

        DataManager<double, data::Current, data::Info> data_nodes;
        Ptr<bloom_filter> seen;

        public:
        OEETracker(Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> s, fun_calc_data_t d, fun_calc_complexity_t c, int bloom_count = 200000) : 
            systematics_manager(s), skeleton_fun(d), complexity_fun(c) {
            
            // emp_assert(s->GetStoreOutside(), "OEE tracker only works with systematics manager where store_outside is set to true");

            data_nodes.New("change");            
            data_nodes.New("novelty");
            data_nodes.New("diversity");
            data_nodes.New("complexity");

            bloom_parameters parameters;

            // How many elements roughly do we expect to insert?
            parameters.projected_element_count = bloom_count ;

            // Maximum tolerable false positive probability? (0,1)
            parameters.false_positive_probability = 0.0001; // 1 in 10000

            if (!parameters)
            {
                std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
            }

            parameters.compute_optimal_parameters();

            //Instantiate Bloom Filter
            seen.New(parameters);

        }

        ~OEETracker() {seen.Delete();}

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

                emp::vector<snapshot_info_t> active(sys_active.size());
                int i = 0;
                for (auto tax : sys_active) {
                    active[i].taxon = tax;
                    active[i].count = tax->GetNumOrgs();
                    i++;
                }
                snapshots.push_back(active);
                snapshot_times.push_back(ud);
                if ((int)snapshots.size() > generation_interval/resolution + 1) {
                    systematics_manager->RemoveBefore(snapshot_times.front() - 1);
                    snapshot_times.pop_front();
                    snapshots.pop_front();
                }
                CalcStats(ud);
            }
        }

        void CalcStats(size_t ud) {
            std::map<SKEL_TYPE, int> coal_set = CoalescenceFilter(ud);
            // std::unordered_set<SKEL_TYPE> next_prev_coal_set;
            int change = 0;
            int novelty = 0;
            double most_complex = 0;
            double diversity = 0;
            if (coal_set.size() > 0) {
                diversity = Entropy(coal_set, [](std::pair<SKEL_TYPE, int> entry){return entry.second;});
            }

            for (auto & tax : coal_set) {
                // std::cout << "Evaluating org id: " << tax->GetID() << "(" <<tax->GetInfo() << ")" << std::endl;
                if (!Has(prev_coal_set, tax.first)) {
                    change++;
                    // std::cout << "Org id: " << tax->GetID() << "(" <<tax->GetInfo() << ") increased change" << std::endl;
                }
                if (!seen->contains(tax.first)) {
                    novelty++;
                    // std::cout << "seen: ";
                    // for (int val : seen) {
                    //     std::cout << val << " ";
                    // }
                    // std::cout << std::endl;
                    seen->insert(tax.first);
                    // std::cout << "Org id: " << tax->GetID() << "(" <<tax->GetInfo() << ") increased novelty" << std::endl;
                }
                double complexity = complexity_fun(tax.first);
                // std::cout << "Complexity: " << complexity << std::endl;
                if (complexity > most_complex) {
                    // std::cout << "Org id: " << tax->GetID() << "(" <<tax->GetInfo() << ") increased complextiy " << complexity << std::endl;
                    most_complex = complexity;
                }
            }

            data_nodes.Get("change").Add(change);
            data_nodes.Get("novelty").Add(novelty);
            data_nodes.Get("diversity").Add(diversity);
            data_nodes.Get("complexity").Add(most_complex);

            // std::cout << change << ' ' << novelty << " " << diversity << " " << most_complex << std::endl;

            std::swap(prev_coal_set, coal_set);
        }

        std::map<SKEL_TYPE, int> CoalescenceFilter(size_t ud) {

            emp_assert(emp::Mod(generation_interval, resolution) == 0, "Generation interval must be a multiple of resolution", generation_interval, resolution);

            std::map<SKEL_TYPE, int> res;

            if ((int)snapshots.size() <= generation_interval/resolution) {
                return res;
            }

            std::set<Ptr<taxon_t>> extant_canopy_roots = systematics_manager->GetCanopyExtantRoots(snapshot_times.front());
            // std::cout << "exteant canpoy roots: ";
            // for (auto t : extant_canopy_roots) {
            //     std::cout << t->GetInfo() << " ";
            // }
            // std::cout << std::endl;
            for ( snapshot_info_t & t : snapshots.front()) {
                if (Has(extant_canopy_roots, t.taxon)) {
                    // std::cout << t.taxon->GetInfo() << " Survived filter" << std::endl;
                    if (!t.taxon->GetData().skeleton) {
                        t.taxon->GetData().skeleton.New(skeleton_fun(t.taxon->GetInfo()));
                    }
                    if (Has(res, *(t.taxon->GetData().skeleton))) {
                        res[*(t.taxon->GetData().skeleton)] += t.count;
                    } else {
                        res[*(t.taxon->GetData().skeleton)] = t.count;
                    }
                }
            }

            return res;
        }


        Ptr<DataNode<double, data::Current, data::Info>> GetDataNode(const std::string & name) {
            return &(data_nodes.Get(name));
        }        

    };

    // Helper function for skeletonization

    // Assumes org is sequence of inst_type
    template <typename ORG_TYPE, typename INST_TYPE>
    emp::vector<INST_TYPE> Skeletonize(const ORG_TYPE & org, const INST_TYPE null_value, std::function<double(const ORG_TYPE&)> fit_fun) {
        emp_assert(org.size() > 0, "Empty org passed to skeletonize");

        emp::vector<INST_TYPE> skeleton;
        double fitness = fit_fun(org);
        ORG_TYPE test_org = ORG_TYPE(org);

        for (size_t i = 0; i < org.size(); i++) {
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

#endif