//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Track genotypes, species, clades, and lineages of organisms in a world.
//
//
//  Options include:
//  * Track lineage yes / no.
//  * Group genotypes yes / no
//  * Group clades (provide a function that determines when a new clade should start)
//  * Prune evolutionary dead-ends (to reduce size of tree).
//  * Backup to file (anything older than a specified level)


#ifndef EMP_EVO_SYSTEMATICS_H
#define EMP_EVO_SYSTEMATICS_H

#include <unordered_map>

#include "../base/Ptr.h"

namespace emp {

  template <typename ORG>
  class Systematics {
  private:
    struct OrgNode {
      size_t id;
      size_t parent_id;
    };

    unordered_map< size_t, Ptr<OrgNode> > node_map;

  public:
    Systematics() : node_map() { ; }
    ~Systematics() {
      for (auto x : node_map) x.second.Delete();
      node_map.clear();
    }

    /// Add information about a new organism; return an id for this new entry.
    size_t AddOrg(size_t parent_id, ORG & org) {
    }

    /// Remove an instance of an organism; track when it's gone.
    size_t RemoveOrg(ORG & org) {
    }

    /// Climb up a lineage...
    size_t ParentID(size_t org_id) {
    }
  };

}

#endif
