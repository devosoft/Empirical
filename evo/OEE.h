//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
#ifndef EMP_OEE_H
#define EMP_OEE_H

#include <set>
#include <algorithm>
#include <iterator>

#include "LineageTracker.h"

namespace emp{
namespace evo{
  template <typename GENOME>
    int ChangeMetric(LineageTracker<GENOME>* lineages, 
		     std::set<int> curr_generation, 
		     std::set<int> prev_generation, int generations){
    
    //Find persistant lineages
    std::set<GENOME> persist = GetPersistLineage(lineages, curr_generation,  generations);
    std::set<GENOME> prev_persist = GetPersistLineage(lineages, prev_generation, generations);
    
    std::set<GENOME> result;
    std::set_difference(persist.begin(), persist.end(), prev_persist.begin(),
			prev_persist.end(), std::inserter(result, result.end()));
    return result.size();
  }

  template <typename GENOME>
    std::set<GENOME> GetPersistLineage(LineageTracker<GENOME>* lineages, 
				       std::set<int> curr_generation,
				       int generations){

    std::set<GENOME> persist;
    for (int id : curr_generation){
      emp::vector<GENOME*> lin = lineages->TraceLineage(id);
      persist.insert(**(lin.begin() + generations));
    }
    
    return persist;
  }


}
}


#endif
