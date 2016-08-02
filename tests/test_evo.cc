#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "../evo/NK.h"
#include "../evo/World.h"
#include "../tools/BitSet.h"
#include "../tools/Random.h"
#include "../evo/EvoStats.h"
#include "../evo/StatsManager.h"

using BitOrg = emp::BitVector;

template <typename ORG>
using MixedWorld = emp::evo::World<ORG, emp::evo::PopulationManager_Base<ORG>>;

TEST_CASE("Test Stats/NK-Grid", "[stats]"){

  // k controls # of hills in the fitness landscape
  const int K = 0;
  const int N = 30;
  const double MUTATION_RATE = .0001;

  const int TOURNAMENT_SIZE = 20;
  const int POP_SIZE = 100;
  const int UD_COUNT = 100;

  emp::Random random(123);
  emp::evo::NKLandscape landscape(N, K, random);

  std::string prefix;
  prefix = "temp/Result-";


  emp::evo::GridWorld<BitOrg, emp::evo::LineagePruned > grid_pop(random);

  grid_pop.ConfigPop(std::sqrt(POP_SIZE), std::sqrt(POP_SIZE));

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  grid_pop.SetDefaultFitnessFun(fit_func);

  // make a stats manager
  emp::evo::StatsManager_AdvancedStats<emp::evo::PopulationManager_Grid<BitOrg>>
      grid_stats (&grid_pop, prefix + "grid.csv");

  grid_stats.SetDefaultFitnessFun(fit_func);


    // Insert default organisms into world
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);

    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    grid_pop.Insert(next_org);
  }

  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the
  // site will flip it's value.
  grid_pop.SetDefaultMutateFun( [MUTATION_RATE, N](BitOrg* org, emp::Random& random) {
    bool mutated = false;
      for (size_t site = 0; site < N; site++) {
        if (random.P(MUTATION_RATE)) {
          (*org)[site] = !(*org)[site];
          mutated = true;
        }
      }
      return mutated;
    } );


  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++) {

    // Run a tournament
    grid_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
            , TOURNAMENT_SIZE, POP_SIZE);

    grid_pop.Update();
    grid_pop.MutatePop();

  }

  std::ifstream correct, test;

  test.open("temp/Result-grid.csv");
  correct.open("test-data/Result-grid.csv");

  std::string line = "";
  std::vector<std::string> file1, file2;

  while(getline(correct, line) ){
      file1.push_back(line);
  }
  while(getline(test, line)){
      file2.push_back(line);
  }

  REQUIRE(file1.size() == file2.size());
  if(file1.size() == file2.size()){
      for(size_t i = 0; i < file1.size(); i++){
          REQUIRE(file1[i] == file2[i]);
      }
  }
}

TEST_CASE("Test-stats-NK-Mixed","[stats]"){
 // k controls # of hills in the fitness landscape
  const int K = 0;
  const int N = 30;
  const double MUTATION_RATE = .0001;

  const int TOURNAMENT_SIZE = 20;
  const int POP_SIZE = 100;
  const int UD_COUNT = 100;

  emp::Random random(123);
  emp::evo::NKLandscape landscape(N, K, random);

  std::string prefix;
  prefix = "temp/Result-";

  // Create World
  MixedWorld<BitOrg> mixed_pop(random);

  std::function<double(BitOrg *)> fit_func =[&landscape](BitOrg * org) { return landscape.GetFitness(*org);};

  mixed_pop.SetDefaultFitnessFun(fit_func);

  // make a stats manager
  emp::evo::StatsManager_DefaultStats<emp::evo::PopulationManager_Base<BitOrg>>
      mixed_stats (&mixed_pop, prefix + "mixed.csv");

  mixed_stats.SetDefaultFitnessFun(fit_func);

  // Insert default organisms into world
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (int j = 0; j < N; j++) next_org[j] = random.P(0.5);

    // looking at the Insert() func it looks like it does a deep copy, so we should be safe in
    // doing this. Theoretically...
    mixed_pop.Insert(next_org);
  }


  // mutation function:
  // for every site in the gnome there is a MUTATION_RATE chance that the
  // site will flip it's value.
  mixed_pop.SetDefaultMutateFun( [MUTATION_RATE, N](BitOrg* org, emp::Random& random) {
    bool mutated = false;
      for (size_t site = 0; site < N; site++) {
        if (random.P(MUTATION_RATE)) {
          (*org)[site] = !(*org)[site];
          mutated = true;
        }
      }
      return mutated;
    } );


  // Loop through updates
  for (int ud = 0; ud < UD_COUNT; ud++) {

    // Keep the best individual.
    //mixed_pop.EliteSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }, 1, 100);
    // Run a tournament for the rest...

    mixed_pop.TournamentSelect([&landscape](BitOrg * org){ return landscape.GetFitness(*org); }
			 , TOURNAMENT_SIZE, POP_SIZE);

    mixed_pop.Update();
    mixed_pop.MutatePop();

  }


  std::ifstream correct, test;

  test.open("temp/Result-mixed.csv");
  correct.open("test-data/Result-mixed.csv");

  std::string line = "";
  std::vector<std::string> file1, file2;

  while(getline(correct, line) ){
      file1.push_back(line);
  }
  while(getline(test, line)){
      file2.push_back(line);
  }
  REQUIRE(file1.size() == file2.size());
  if(file1.size() == file2.size()){
      for(size_t i = 0; i < file1.size(); i++){
          REQUIRE(file1[i] == file2[i]);
      }
  }

}
