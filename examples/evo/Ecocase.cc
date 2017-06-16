//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h with an NK landscape.

#include <iostream>

#include "../../evo/World.h"
#include "../../tools/math.h"
#include "../../tools/Random.h"

constexpr double radius = 0.01;
constexpr size_t MAX_GENS = 2000;
constexpr double MUT_SIZE = 0.01;
constexpr size_t POP_SIZE = 200;

constexpr double radius2 = radius * radius;

// Reward anything close to (0,0)
double Fit_Main(double x, double y) {
  double dist = x*x + y*y;
  if (dist < radius2) return 1.0;
  return 0.0;
}

// Promote close to x==y
double Fit_Help(double x, double y) {
  double diff = x-y;
  return (diff > -0.01 && diff < 0.01) ? 1.0 : 0.0;
}

// Promote close to (1,y)
double Fit_Hurt(double x, double y) {
  return x;
}

struct Org {
  double x;
  double y;
  double density; // How many neigbors are nearby?
  size_t wins;

  Org(double _x, double _y) : x(_x), y(_y), density(0.0), wins(0) { ; }
};


// Initialize the popoulation at the start of each generation.
template <typename T>
void Init(T & pop) {

  for (size_t i = 0; i < POP_SIZE; i++) {
    pop[i].density = 0.0;
    pop[i].wins = 0;

    for (size_t j = 0; j < i; j++) {
      const double x_dist = 2 * (pop[i].x - pop[j].x);
      const double y_dist = 2 * (pop[i].y - pop[j].y);
      const double val = emp::Min(1.0 / (x_dist * x_dist + y_dist * y_dist), 20000.0);
      pop[i].density += val;
      pop[j].density += val;
    }
  }

}



int main(int argc, char* argv[])
{
  emp::Random random;

  int counter = 0;
  int NUM_RUNS = 100;

  for (int n = 0; n < NUM_RUNS; n++) {
    std::cout << "Run " << n << std::endl;

    emp::evo::EAWorld<Org, emp::evo::FitCacheOff> pop(random, "EcoWorld");

    // Build a random initial population
    for (uint32_t i = 0; i < POP_SIZE; i++) {
      Org org( random.GetDouble(), random.GetDouble() );
      pop.Insert(org);
    }

    pop.SetDefaultMutateFun( [](Org* org, emp::Random& random) {
    	org->x = emp::ToRange(org->x + random.GetDouble(-MUT_SIZE, MUT_SIZE), 0.0, 1.0);
    	org->y = emp::ToRange(org->y + random.GetDouble(-MUT_SIZE, MUT_SIZE), 0.0, 1.0);
    	return true;
    } );

    // emp::vector<std::function<double(Org*)>> fit_funs(4);
    emp::vector<std::function<double(Org*)>> fit_funs;
    // fit_funs.push_back( [](Org* org){ return Fit_Main(org->x, org->y); } );
    fit_funs.push_back( [](Org* org){ return Fit_Help(org->x, org->y); } );
    fit_funs.push_back( [](Org* org){ return Fit_Hurt(org->x, org->y); } );
    fit_funs.push_back( [](Org* org){ return -(org->density); } );

    std::function<double(Org*)> main_fun = [](Org* org){ return Fit_Main(org->x, org->y); };

    emp::vector<double> probs = { 1.0, 1.0, 1.0, 1.0 };

    // Loop through updates
    for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
      Init(pop);
      // pop.EcocaseSelect(fit_funs, probs, 0.6, POP_SIZE);
      // pop.EcocaseSelect(fit_funs, probs, 1.0, POP_SIZE);
      pop.EcoSelect(main_fun, fit_funs, 10, 5, POP_SIZE);
      // pop.TournamentSelect(main_fun, 5, POP_SIZE);

      pop.Update();
//       std::cout << "Gen " << (ud+1) << " : (" << (pop[0].x) << "," << (pop[0].y) << ") : "
// 		<< "  Fit_Main() = " << fit_funs[0]( &(pop[0]) )
// 		<< "  Fit_Help() = " << fit_funs[1]( &(pop[0]) )
// 		<< "  Fit_Hurt() = " << fit_funs[2]( &(pop[0]) )
// 		<< "  Spread = " << fit_funs[3]( &(pop[0]) )
// 		<< std::endl;
      pop.MutatePop(0);
    }


    // Print out the whole population:
    for (size_t i = 0; i < POP_SIZE; i++) {
      if (main_fun( &(pop[i]) )) {
      	counter++;
      	break;
      }
    }
//       std::cout << "Org " << i << " : (" << pop[i].x << "," << pop[i].y << ") : "
// 		<< "  Fit_Main() = " << fit_funs[0]( &(pop[i]) )
// 		<< "  Fit_Help() = " << fit_funs[1]( &(pop[i]) )
// 		<< "  Fit_Hurt() = " << fit_funs[2]( &(pop[i]) )
// 		<< "  Spread = " << fit_funs[3]( &(pop[i]) )
// 		<< std::endl;

  }

  std::cout << "Num successes: " << counter << std::endl;
}
