//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This is an example file, evolving AvidaGP organisms to sort numbers.

#include <iostream>
#include <unordered_map>

#include "../../base/vector.h"
#include "../../evo/World.h"
#include "../../hardware/AvidaGP.h"
#include "../../tools/Random.h"

constexpr size_t POP_SIZE = 200;
constexpr size_t GENOME_SIZE = 100;
constexpr size_t EVAL_TIME = 500;
constexpr size_t UPDATES = 1000;
constexpr size_t TOURNY_SIZE = 4;

constexpr size_t SORT_VALS = 10;

constexpr size_t sort_pairs = SORT_VALS * (SORT_VALS - 1) / 2;

std::vector<double> rand_inputs;

int main()
{
  emp::Random random;
  emp::evo::EAWorld<emp::AvidaGP> world(random, "AvidaWorld");

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    emp::AvidaGP cpu;
    cpu.PushRandom(random, GENOME_SIZE);
    world.Insert(cpu);
  }

  // Setup the mutation function.
  world.SetDefaultMutateFun( [](emp::AvidaGP* org, emp::Random& random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org->RandomizeInst(pos, random);
      }
      return (num_muts > 0);
    } );

  // Setup the fitness function.
  std::function<double(emp::AvidaGP*)> fit_fun =
    [&random, &world](emp::AvidaGP * org) {
      emp_assert(rand_inputs.size() == SORT_VALS);
      org->ResetHardware();
      for (size_t i = 0; i < SORT_VALS; i++) org->SetInput((int)i, rand_inputs[i]);

      // Let the CPU process the run.
      org->Process(EVAL_TIME);

      // Collect the outputs...
      const std::unordered_map<int,double> & outputs = org->GetOutputs();

      // Determine WHERE each input value was moved to; initialize to -1 in case it wasn't output.
      std::unordered_map<double,int> output_pos;
      for (double rand_i : rand_inputs) output_pos[rand_i] = -1;
      for (const auto & out_pair : outputs) output_pos[out_pair.second] = out_pair.first;

      // Now evaluate the outputs.
      double score = 0.0;
      for (size_t i = 1; i < SORT_VALS; i++) {
        const double val1 = rand_inputs[i];
        const int pos1 = output_pos[val1];
        for (size_t j = 0; j < i; j++) {
          const double val2 = rand_inputs[j];
          const int pos2 = output_pos[val2];

          // If either number is missing, ordering fails.
          if (pos1 < 0 || pos2 < 0) { org->PushTrait(0.0); continue; }

          // If numbers are in the wrong order, ordering fails.
          if ( (pos1 < pos2) != (val1 < val2) ) { org->PushTrait(0.0); continue; }

          // Ordering must be successful!
          org->PushTrait(1.0);
          score += 1.0;
        }
      }
      return score;
    };

  // Setup the "hint" functions...
  emp::vector< std::function<double(emp::AvidaGP*)> > fit_set;
  // Create a hint for the ordering of each pair of functions.
  for (size_t i=0; i < sort_pairs; i++) {
    fit_set.push_back( [i](emp::AvidaGP * org) {
      return org->GetTrait(i);
    });
  }


  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Keep the best individual. (and initialize all traits)
    world.EliteSelect(fit_fun, 1, 1);

    // Run a tournament for each spot.
    // world.TournamentSelect(fit_fun, TOURNY_SIZE, POP_SIZE-1);
    world.LexicaseSelect(fit_set, POP_SIZE-1);
    // world.EcoSelect(fit_fun, fit_set, 100, TOURNY_SIZE, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;

    // Mutate all but the first organism.
    world.MutatePop(1);
  }

  return 0;
}
