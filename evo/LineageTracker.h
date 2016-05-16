//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef EMP_LINEAGE_TRACKER_H
#define EMP_LINEAGE_TRACKER_H

#include <map>
#include <set>
#include "../tools/vector.h"

namespace emp{
namespace evo{

  // Class to keep track of lineages
  // Maintains record of all genomes that ever existed, which organisms
  // they belonged to, and which organisms were the parents of which
  template <typename GENOME>
  class LineageTracker {
  public:
    std::set<GENOME> genomes;
    std::map<int, GENOME*> org_to_genome;
    std::map<int, int> parents;
    int next = 0; //-1 indicates no parent

    LineageTracker() {
      this->parents[-1] = 0;
    }
    ~LineageTracker() {
      //for (GENOME g : genomes) delete &g;
    }

    // Add an organism to the tracker - org is the genome of the organism
    // and parent is the id of the parent. The lineage tracker is in charge
    // of assigning ids, and will return an int representing the id of the
    // organism you added
    int AddOrganism(GENOME org, int parent) {
      int id = this->next++;
      std::pair<typename std::set<GENOME>::iterator, bool> ret;
      ret = genomes.insert(org);
      typename std::set<GENOME>::iterator it = ret.first;
      GENOME* genome = (GENOME*)&(*it);
      this->org_to_genome[id] = genome;
      this->parents[id] = parent;

      return id;
    }

    // Return a vector containing the genomes of an organism's ancestors
    emp::vector<GENOME*> TraceLineage(int org_id) {
      emp::vector<GENOME*> lineage;

      while(org_id) {
        lineage.push_back(this->org_to_genome[org_id]);
        org_id = this->parents[org_id];
      }

      return lineage;

    }

    //Return a vector containing the IDs of an oraganism's ancestors
    emp::vector<int> TraceLineageIDs(int org_id) {
      emp::vector<GENOME*> lineage;

      while(org_id) {
        lineage.push_back(org_id);
        org_id = this->parents[org_id];
      }

      return lineage;

    }

  };


}
}

#endif
