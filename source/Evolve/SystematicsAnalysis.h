/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 * 
 * This file contains extra analysis tools to use with systematics managers that
 * have non-null DATA_TYPES.
**/

#include "Systematics.h"


// Mutation info functions. Assumes each taxon has a struct containing an unordered map
// with keys that are strings indicating types of mutations and keys that are numbers
// indicating the number of that type of mutation that occurred to make this taxon from
// the parent.

namespace emp {

    template <typename taxon_t>
    int CountMuts(Ptr<taxon_t> taxon, std::string type="substitution") {
        int count = 0;

        while (taxon) {
            count += taxon->GetData().mut_counts[type];
            taxon = taxon->GetParent();            
        }

        return count;
    }

    template <typename taxon_t>
    int CountDeleteriousSteps(Ptr<taxon_t> taxon) {
        int count = 0;
        Ptr<taxon_t> parent = taxon->GetParent();

        while (parent) {
            if (taxon->GetData().fitness < parent->GetData().fitness) {
                count++;
            }
            taxon = parent;            
            parent = taxon->GetParent();               
        }

        return count;
    }

    template <typename taxon_t>
    int CountUniquePhenotypes(Ptr<taxon_t> taxon) {
        int count = 1; // Start with current phenotype
        Ptr<taxon_t> parent = taxon->GetParent();

        while (parent) {
            if (taxon->GetData().phenotype != parent->GetData().phenotype) {
                count++;
            }
            taxon = parent;            
            parent = taxon->GetParent();               
        }

        return count;
    }

};