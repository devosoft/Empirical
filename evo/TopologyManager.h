//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//

#ifndef EMP_EVO_TOPOLOGY_MANAGER_H
#define EMP_EVO_TOPOLOGY_MANAGER_H

#include "../tools/random_utils.h"
#include "PopulationManager.h"
#include "PopulationIterator.h"

namespace emp {
namespace evo {

    template <typename ORG=int>
    class TopologyManager_Base {
    protected:
        using ptr_t = ORG *;
        emp::vector<ORG *> pop;

        Random * random_ptr;

    public:
        TopologyManager_Base() { ; }
        ~TopologyManager_Base() { ; }


    };

}
}


#endif
