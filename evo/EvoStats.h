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

struct MLandscape{
    double benefit_avg;
    double neutral_avg;
    double det_avg;

    MLandscape(){ benefit_avg = 0; neutral_avg = 0; det_avg = 0;}
};

namespace emp{
namespace evo{

  
  // Calculates the Non-Inferiority of a given container
template <typename ORG, typename WORLD>
  double NonInf(std::function<double(ORG * org)> fit_fun, WORLD & orgs){
    double fittest = fit_fun(&(*(orgs.begin())));
    vector<double> org_fit;

    for (auto org : orgs){
      double fitness = fit_fun(&org);
      org_fit.push_back(fitness);

      if (fitness > fittest){ fittest = fitness; }
    }

    double non_total = 0;
    double num_orgs = 0;

    for(auto el : org_fit){
        num_orgs++;
        non_total += (el / fittest);
    }

    return non_total / num_orgs;
  }

  // Calculates the benefitial, neutral, and detremental mutational landscapes
template <typename ORG, typename WORLD>
  MLandscape  MutLandscape(std::function<double(ORG * org)> fit_fun, WORLD & orgs){
      double  mut_ben = 0, mut_det = 0, mut_neu = 0;
      int total_orgs = 0;

      for (auto org : orgs){
          total_orgs++; //get a total count of orgs in current population

          int benefit = 0, neutral = 0, detremental = 0;
          double fitness = fit_fun(&org);
          ORG test = org;

          for(int i = 0; i < org.size(); i++){
              test[i] = !test[i]; //invert genome

              if(fit_fun(&test) > fitness){ benefit++; }
              else if(fit_fun(&test) == fitness){ neutral++; }
              else{ detremental++; }

              test[i] = !test[i]; //revert genome
          }
          
          double org_avg_b = benefit, org_avg_n = neutral, org_avg_d = detremental;

          org_avg_b = org_avg_b / org.size();
          org_avg_n = org_avg_n / org.size();
          org_avg_d = org_avg_d / org.size();

          mut_ben += org_avg_b;
          mut_neu += org_avg_n;
          mut_det += org_avg_d;
      }
      
      MLandscape data;

      data.benefit_avg = mut_ben / total_orgs;
      data.neutral_avg = mut_neu / total_orgs;
      data.det_avg = mut_det / total_orgs;

      return data;    
  }


}
}

#endif

//Base class outputs most recent
//Write derived class
