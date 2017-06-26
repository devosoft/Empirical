//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Track the lineages of organisms in a world.
//
//
//  Options include:
//  * Prune evolutionary dead-ends (to reduce size of tree).
//  * Backup to file (anything older than a spcified level)


#ifndef EMP_LINEAGE_TRACKER_H
#define EMP_LINEAGE_TRACKER_H

#include <unordered_map>

#include "../base/Ptr.h"

namespace emp {

  template <typename ORG>
  class LineageTracker {
  private:
    struct Node {
      size_t id;
      size_t parent_id;
    };

    unordered_map< size_t, Ptr<Node> > node_map;

  public:
    LineageTracker() : node_map() { ; }
    ~LineageTracker() {
      for (auto x : node_map) x.second.Delete();
      node_map.clear();
    }

    /// Add information about a new organism; return id for this new entry.
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
