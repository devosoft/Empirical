//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

//  This file contains functions for calculating various statistics about
//  a population.
//

#ifndef EMP_EVO_STATS_H
#define EMP_EVO_STATS_H

#include <type_traits>
#include <map>
#include <cmath>
#include <vector>
#include <set>
#include "../tools/vector.h"


namespace emp{
namespace evo{

  //Calculates Shannon Entropy of the members of the container passed
  template <typename C>
  double ShannonDiversity(C elements) {

    //Count number of each value present
    std::map<typename C::value_type, int> counts;
    for (auto element : elements) {
      if (counts.find(element) != counts.end()) {
	       counts[element]++;
      } else {
	       counts[element] = 1;
      }
    }

    //Shannon entropy calculation
    double result = 0;
    for (auto element : counts) {
      double p = double(element.second)/elements.size();
      result +=  p * log2(p);
    }

    return -1 * result;
  }

  //Calculates number of unique elements in the container passed
  template <typename C>
  int Richness(C elements) {
    //Converting to a set will remove duplicates leaving only unique values
    std::set<typename C::value_type> unique_elements(elements.begin(),
						     elements.end());
    return unique_elements.size();
  }

}
}

#endif

//Base class outputs most recent
//Write derived class
