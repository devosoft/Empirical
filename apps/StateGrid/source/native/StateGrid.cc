// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "../SGWorld.h"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 10000;

int main()
{
  emp::Random random;
  SGWorld world(random, "AvidaWorld");
  emp::StateGrid state_grid;

  state_grid.AddState(-1, '-', -0.5, "None",          "Empty space; poisonous.");
  state_grid.AddState( 0, '.',  0.0, "Consumed Food", "Previously had sustanance for an organism.");
  state_grid.AddState( 1, '#', +1.0, "Food",          "Sustanance to an org.");
  state_grid.AddState( 2, 'x',  0.0, "Consumed Edge", "Edge marker; previously had food.");
  state_grid.AddState( 3, 'X', +1.0, "Edge",          "Edge marker with food.");

  state_grid.Load("state_grids/islands_50x50.cfg");

  // When an organism is added to the world, supply it with a state grid.
  world.OnOrgPlacement( [&state_grid, &world, &random](size_t pos){
      world.GetOrg(pos).SetStateGrid(state_grid);
      // if (pos && random.P(0.1)) world.GetOrg(pos).GetSGStatus().Randomize(state_grid, random);
    } );

  world.SetWellMixed(true);

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    SGOrg cpu(&(world.inst_lib));
    cpu.SetStateGrid(state_grid);
    cpu.PushRandom(random, GENOME_SIZE);
    world.Inject(cpu.GetGenome());
  }

  // Setup the mutation function.
  world.SetMutFun( [](SGOrg & org, emp::Random & random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org.RandomizeInst(pos, random);
      }
      return num_muts;
    } );

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Progress output...
    std::cout << "Update " << ud;

    // Keep the best individual.
    EliteSelect(world, 1, 1);

    std::cout << "  fitness[0] = " << world[0].GetScore()
              << std::endl;

    // Run a tournament for the rest...
    TournamentSelect(world, 4, POP_SIZE-1);

    // Put new organisms is place.
    world.Update();

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  std::cout << "Final Fitness: " << world.CalcFitnessID(0) << std::endl;
  world[0].GetStateGrid().Print();

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
