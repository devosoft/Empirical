#ifndef EMP_OEE_STATS_H
#define EMP_OEE_STATS_H

#include "Systematics.h"
#include "base/vector.h"
#include "base/Ptr.h"
#include "tools/set_utils.h"
#include "tools/vector_utils.h"

#include <deque>

namespace emp {


    template <typename ORG, typename ORG_INFO, typename DATA_STRUCT=emp::datastruct::no_data>
    class OEETracker {
        private:
        using taxon_t = Taxon<ORG_INFO, DATA_STRUCT>;
        using hash_t = typename Ptr<taxon_t>::hash_t;
        using fun_calc_complexity_t = std::function<double(Ptr<taxon_t>)>;

        struct snapshot_info_t {
            Ptr<taxon_t> taxon = nullptr; // This is what the systematics manager has
            int count; // Count of this taxon at time of snapshot
            // bool operator==(const snapshot_info_t & other) const {return other.taxon == taxon;}
        };

        std::deque<emp::vector<snapshot_info_t>> snapshots;
        Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> systematics_manager;

        std::unordered_set<Ptr<taxon_t>, hash_t> prev_coal_set;
        std::set<ORG_INFO> seen;

        fun_calc_complexity_t complexity_fun;
        int generation_interval = 10;
        int resolution = 10;

        DataManager<double, data::Current, data::Info> data_nodes;

        public:
        OEETracker(Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> s, fun_calc_complexity_t c) : 
            systematics_manager(s), complexity_fun(c) {
            
            // emp_assert(s->GetStoreOutside(), "OEE tracker only works with systematics manager where store_outside is set to true");

            data_nodes.New("change");            
            data_nodes.New("novelty");
            data_nodes.New("diversity");
            data_nodes.New("complexity");
        }

        int GetResolution() const {return resolution;}
        int GetGenerationInterval() const {return generation_interval;}

        void SetResolution(int r) {resolution = r;}
        void SetGenerationInterval(int g) {generation_interval = g;}

        void Update(size_t ud) {
            if (Mod((int)ud, resolution) == 0) {
                auto & sys_active = systematics_manager->GetActive();
                emp::vector<snapshot_info_t> active(sys_active.size());
                int i = 0;
                for (auto tax : sys_active) {
                    active[i].taxon = tax;
                    active[i].count = tax->GetNumOrgs();
                    i++;
                }
                snapshots.push_back(active);
                if (snapshots.size() > generation_interval/resolution + 1) {
                    snapshots.pop_front();
                }
                CalcStats();
            }
        }

        void CalcStats() {
            emp::vector<snapshot_info_t> coal_set = CoalescenceFilter();
            std::unordered_set<Ptr<taxon_t>, hash_t> next_prev_coal_set;
            int change = 0;
            int novelty = 0;
            double most_complex = 0;
            double diversity = 0;
            if (coal_set.size() > 0) {
                diversity = Entropy(coal_set, [](snapshot_info_t t){return t.count;});
            }

            for (snapshot_info_t tax : coal_set) {
                // std::cout << "Evaluating org id: " << tax->GetID() << "(" <<tax->GetInfo() << ")" << std::endl;
                if (!Has(prev_coal_set, tax.taxon)) {
                    change++;
                    // std::cout << "Org id: " << tax->GetID() << "(" <<tax->GetInfo() << ") increased change" << std::endl;
                }
                if (!Has(seen, tax.taxon->GetInfo())) {
                    novelty++;
                    // std::cout << "seen: ";
                    // for (int val : seen) {
                    //     std::cout << val << " ";
                    // }
                    // std::cout << std::endl;
                    seen.insert(tax.taxon->GetInfo());
                    // std::cout << "Org id: " << tax->GetID() << "(" <<tax->GetInfo() << ") increased novelty" << std::endl;
                }
                double complexity = complexity_fun(tax.taxon);
                // std::cout << "Complexity: " << complexity << std::endl;
                if (complexity > most_complex) {
                    // std::cout << "Org id: " << tax->GetID() << "(" <<tax->GetInfo() << ") increased complextiy " << complexity << std::endl;
                    most_complex = complexity;
                }
                next_prev_coal_set.insert(tax.taxon);
            }

            data_nodes.Get("change").Add(change);
            data_nodes.Get("novelty").Add(novelty);
            data_nodes.Get("diversity").Add(diversity);
            data_nodes.Get("complexity").Add(most_complex);

            // std::cout << change << ' ' << novelty << " " << diversity << " " << most_complex << std::endl;

            prev_coal_set = next_prev_coal_set;
        }

        emp::vector<snapshot_info_t> CoalescenceFilter() {

            emp_assert(emp::Mod(generation_interval, resolution) == 0, "Generation interval must be a multiple of resolution", generation_interval, resolution);

            emp::vector<snapshot_info_t> res;

            if (snapshots.size() <= generation_interval/resolution) {
                return res;
            }

            for ( snapshot_info_t & t : snapshots.front()) {
                if (Has(systematics_manager->GetActive(), t.taxon) || Has(systematics_manager->GetAncestors(), t.taxon)) {
                    res.push_back(t);
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
            if (new_fitness >= fitness) {
                skeleton.push_back(null_value);
            } else {
                skeleton.push_back(org[i]);
            }
            test_org[i] = org[i];
        }

        return skeleton;
    }

}

#endif