#ifndef EMP_OEE_STATS_H
#define EMP_OEE_STATS_H

#include "Systematics.h"
#include "base/vector.h"
#include "base/Ptr.h"

namespace emp {

    template <typename ORG, typename ORG_INFO, typename DATA_STRUCT=emp::datastruct::no_data>
    class OEETracker {
        private:
        using taxon_t = Taxon<ORG_INFO, DATA_STRUCT>;
        using hash_t = typename Ptr<taxon_t>::hash_t;
        using taxa_set_t = std::unordered_set< Ptr<taxon_t>, hash_t >;
        using fun_calc_complexity_t = std::function<double(Ptr<taxon_t>)>;

        emp::vector<taxa_set_t> snapshots;
        Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> systematics_manager;

        taxa_set_t prev_coal_set;
        taxa_set_t seen;

        fun_calc_complexity_t complexity_fun;
        size_t generation_interval;
        int resolution = 10;

        DataManager<double, data::Current, data::Info> data_nodes;

        public:
        OEETracker(Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> s, fun_calc_complexity_t c) : 
            systematics_manager(s), complexity_fun(c) {
            
            emp_assert(s->GetStoreOutside(), "OEE tracker only works with systematics manager where store_outside is set to true");

            data_nodes.New("change");            
            data_nodes.New("novelty");
            data_nodes.New("diversity");
            data_nodes.New("complexity");
        }

        int GetResolution() const {return resolution;}
        size_t GetGenerationInterval() const {return generation_interval;}

        void SetResolution(int r) const {resolution = r;}
        void SetGenerationInterval(size_t g) const {generation_interval = g;}

        void Update(size_t ud) {
            taxa_set_t active = systematics_manager->GetActive();
            snapshots.push_back(active);
            if (Mod((int)ud, resolution) == 0) {
                CalcStats();
            }
        }

        void CalcStats() {
            taxa_set_t coal_set = CoalescenceFilter();
            int change = 0;
            int novelty = 0;
            double most_complex = 0;
            double diversity = ShannonEntropy(coal_set);
            for (Ptr<taxon_t> tax : coal_set) {
                if (!Has(prev_coal_set, tax)) {
                    change++;
                }
                if (!Has(seen, tax)) {
                    novelty++;
                    seen.insert(tax);
                }
                double complexity = complexity_fun(tax);
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

        taxa_set_t CoalescenceFilter() {
            taxa_set_t include_set;
            for (Ptr<taxon_t> tax : snapshots[0]) {
                Ptr<taxon_t> ancestor = GetAncestor(tax);
                if (ancestor) {
                    include_set.insert(ancestor);
                }
            }
            return include_set;
        }

        Ptr<taxon_t> GetAncestor(Ptr<taxon_t> t) {
            for (size_t i = 0; i < generation_interval; i++) {
                t = t->GetParent();
                if (!t) {
                    return nullptr;
                }
            }
            return t;
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
            if (fit_fun(test_org) >= fitness) {
                skeleton.push_back(null_value);
            } else {
                skeleton.push_back(org[i]);
            }
        }

        return skeleton;
    }

}

#endif