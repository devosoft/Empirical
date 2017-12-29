// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "tools/math.h"

#include "../SGPatches.h"
#include "../SGWorld.h"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 400;       // Total population size.
constexpr size_t ELITE_SIZE = 10;      // Top how many organisms should move to next generation?
constexpr size_t ELITE_COPIES = 1;     // How many copies of each elite organism should be made?
constexpr size_t GENOME_SIZE = 50;     // How long of a genome should we be using?
constexpr size_t UPDATES = 1000;       // How many generations to run?
constexpr size_t CPU_TIME = 5000;      // How many CPU cycles to process for?
constexpr double GOOD_BAD_RATIO = 1.0; // Value of going to a good square vs avoiding a bad square.

constexpr size_t ELITE_TOTAL = ELITE_SIZE * ELITE_COPIES;

int main()
{
  emp::Random random;
  SGWorld world(random, "AvidaWorld");
  auto & state_grid = world.GetStateGrid();

  world.SetWellMixed(true);


  // Setup a set of HINT functions.
  emp::vector< std::function<double(const SGOrg &)> > hint_funs;
  for (size_t h = 0; h < state_grid.GetSize(); h++) {
    int target_state = state_grid.GetState(h);
    if (target_state != -1 && target_state != 1) continue;
    size_t target_x = h % state_grid.GetWidth();
    size_t target_y = h / state_grid.GetWidth();

    hint_funs.push_back( [target_x,target_y,target_state](const SGOrg & org) {
      return org.GetSGStatus().WasAt(target_x, target_y) ? target_state : 0.0;
    });
  }



  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    SGOrg cpu(&(world.inst_lib));
    cpu.PushRandom(random, GENOME_SIZE);
    world.Inject(cpu.GetGenome());
  }

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Progress output...
    std::cout << "Update " << ud;

    // Run all of the organisms to trace their movement.
    world.ResetHardware();
    world.Process(CPU_TIME);

    // Periodically, provide the status of the best organism.
    if (ud % 100 == 0) {
      std::cout << std::endl;
      world[0].GetSGStatus().PrintHistory(world.GetStateGrid());
    }

    // Keep the best individual.
    EliteSelect(world, ELITE_SIZE, ELITE_COPIES);

    std::cout << "  fitness[0] = " << world.CalcFitnessID(0)
              << std::endl;

    // // Run a tournament for the rest...
    // TournamentSelect(world, 4, POP_SIZE-ELITE_TOTAL);

    // Run Lexicase selection for the rest...
    LexicaseSelect(world, hint_funs, POP_SIZE-ELITE_TOTAL);

    // Put new organisms is place.
    world.Update();

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  std::cout << "Final Fitness: " << world.CalcFitnessID(0) << std::endl;
  world.ResetHardware();
  world.Process(CPU_TIME);
  world[0].GetSGStatus().PrintHistory(world.GetStateGrid());

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
