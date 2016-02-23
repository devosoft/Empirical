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
		   LineageTracker<GENOME>* prev_lineages, int generations){
    
    //Find persistant lineages
    std::set<GENOME> persist = GetPersistLineage(lineages, generations);
    std::set<GENOME> prev_persist = GetPersistLineage(prev_lineages, generations);
    
    std::set<GENOME> result;
    std::set_difference(persist.begin(), persist.end(), prev_persist.begin(),
			prev_persist.end(), std::inserter(result, result.end()));
    return result.size();
  }

  template <typename GENOME>
    std::set<GENOME> GetPersistLineage(LineageTracker<GENOME>* lineages, 
				       int generations){

    std::set<GENOME> persist;
    for (int id : lineages->current){
      emp::vector<GENOME*> lin = lineages->TraceLineage(id);
      persist.insert(**(lin.begin() + generations));
    }
    
    return persist;
  }


}
}


#endif
