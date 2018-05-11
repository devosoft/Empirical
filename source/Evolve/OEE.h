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

        // TODO: These probably can't just be pointers  - they could technically
        // get deleted by systematics before the OEE stuff is done with them.
        std::unordered_set<Ptr<taxon_t>, hash_t> prev_coal_set;

        fun_calc_complexity_t complexity_fun;
        std::set<taxon_t> seen;
        size_t generation_interval;
        int resolution = 10;

        DataManager<double, data::Current, data::Info> data_nodes;

        public:
        OEETracker(Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> s, fun_calc_complexity_t c) : 
            systematics_manager(s), complexity_fun(c) {
            
            data_nodes.New("change");            
            data_nodes.New("novelty");
            data_nodes.New("diversity");
            data_nodes.New("complexity");
        }

        void Update(size_t ud) {
            taxa_set_t active = systematics_manager->GetActive();
            snapshots.push_back(active);
            if (Mod((int)ud, resolution) == 0) {
                CalcStats();
            }
        }

        void CalcStats() {
            std::unordered_set<Ptr<taxon_t>, hash_t > coal_set = CoalescenceFilter();
            int change = 0;
            int novelty = 0;
            double most_complex = 0;
            double diversity = ShannonEntropy(coal_set);
            for (Ptr<taxon_t> tax : coal_set) {
                if (!Has(prev_coal_set, tax)) {
                    change++;
                }
                if (!Has(seen, *tax)) {
                    novelty++;
                    seen.insert(*tax);
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

        std::unordered_set<Ptr<taxon_t>, hash_t > CoalescenceFilter() {
            std::unordered_set<Ptr<taxon_t>, hash_t > include_set;
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

}

#endif