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
        emp::vector<taxa_set_t> snapshots;
        Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> systematics_manager;

        size_t generation_interval;
        size_t resolution;

        public:
        OEETracker(Ptr<Systematics<ORG, ORG_INFO, DATA_STRUCT>> s) : 
            systematics_manager(s) {;}

        void Update(size_t ud) {
            taxa_set_t active = systematics_manager->GetActive();
            snapshots.push_back(active);
        }

        std::unordered_set<Ptr<taxon_t> > CoalescenceFilter() {
            std::unordered_set<Ptr<taxon_t> > include_set;
            for (Ptr<taxon_t> tax : snapshots[0]) {
                Ptr<taxon_t> ancestor = GetAncestor(t);
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

    };

}

#endif