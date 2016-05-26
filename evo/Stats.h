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
#include "World.h"

namespace emp{
namespace evo{

  //Calculates Shannon Entropy of the members of the container passed
  template <typename C, class = typename C::value_type >
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

  //Calculates Shannon Entropy of the members of a world
  template <typename ORG, typename... MANAGERS>
  double ShannonDiversity(emp::evo::World<ORG, MANAGERS...> & elements) {

    //Count number of each value present
    std::map<ORG, int> counts;
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
      double p = double(element.second)/elements.GetSize();
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

  template <typename ORG, typename C, class = typename C::value_type >
  double MaxFitness(std::function<double(ORG * org)> fit_fun, C orgs){
    double fittest = fit_fun(&orgs[0]);
    for (auto org : orgs){
      double fitness = fit_fun(&org);
      if (fitness > fittest){
        fittest = fitness;
      }
    }
    return fittest;
  }

  template <typename ORG, typename... MANAGERS>
  double MaxFitness(std::function<double(ORG * org)> fit_fun, World<ORG, MANAGERS...> & orgs){
    double fittest = fit_fun(&(*(orgs.begin())));
    for (auto org : orgs){
      double fitness = fit_fun(&org);
      if (fitness > fittest){
        fittest = fitness;
      }
    }
    return fittest;
  }

template <typename ORG, typename... MANAGERS>
  double AverageFitness(std::function<double(ORG * org)> fit_fun, World<ORG, MANAGERS...> & orgs){
    double cumulative_fitness = 0;
    double num_orgs = 0;
    for (auto org : orgs){
        ++num_orgs;
        cumulative_fitness += fit_fun(&org);
    }
    return (cumulative_fitness / num_orgs);
  }

  template <typename ORG, typename... MANAGERS>
  double NonInf(std::function<double(ORG * org)> fit_fun, World<ORG, MANAGERS...> & orgs){
    double fittest = fit_fun(&(*(orgs.begin())));
    vector<double> org_fit;
    for (auto org : orgs){
      double fitness = fit_fun(&org);
      org_fit.push_back(fitness);
      if (fitness > fittest){
        fittest = fitness;
      }
    }

    std::cout<<"Max Fitness: "<<fittest<<std::endl;
    double non_total = 0;
    double num_orgs = 0;

    for(auto el : org_fit){
        std::cout<<"Org Fitness"<<el<<std::endl;
        num_orgs++;
        non_total += (el / fittest);
    }
    return non_total / num_orgs;
  }


}
}

#endif

//Base class outputs most recent
//Write derived class
