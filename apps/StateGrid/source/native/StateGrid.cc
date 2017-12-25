// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "../SGPatches.h"
#include "../SGWorld.h"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 400;    // Total population size.
constexpr size_t ELITE_SIZE = 10;   // Top how many organisms should move to next generation?
constexpr size_t ELITE_COPIES = 1;  // How many copies of each elite organism should be made?
constexpr size_t GENOME_SIZE = 50;  // How long of a genome should we be using?
constexpr size_t UPDATES = 1000;
constexpr size_t NUM_HINTS = 1000;

constexpr size_t ELITE_TOTAL = ELITE_SIZE * ELITE_COPIES;

int main()
{
  emp::Random random;
  SGWorld world(random, "AvidaWorld");

  world.SetWellMixed(true);

  // Setup a set of hint functions.
  emp::vector< std::function<double(const SGOrg &)> >  hint_funs(NUM_HINTS);
  for (size_t h = 0; h < NUM_HINTS; h++) {
    emp::BitVector target_sites = RandomBitVector(random, world.GetStateGrid().GetSize(), 0.01);
    emp::BitVector good_sites = target_sites & world.GetStateGrid().IsState(1);
    emp::BitVector bad_sites = target_sites & world.GetStateGrid().IsState(-1);
    hint_funs[h] = [good_sites, bad_sites](const SGOrg & org) {
      emp::BitVector visited_sites = org.GetVisited();
      size_t good_visits = (good_sites & visited_sites).CountOnes();
      size_t bad_visits = (bad_sites & visited_sites).CountOnes();
      return good_visits * 2 - bad_visits;
    };
  }

  // emp::BitVector good_sites = world.GetStateGrid().IsState(1);
  // emp::BitVector bad_sites = world.GetStateGrid().IsState(-1);
  // auto fit_fun = [good_sites, bad_sites](const SGOrg & org) {
  //   emp::BitVector visited_sites = org.GetVisited();
  //   double good_visits = (good_sites & visited_sites).CountOnes();
  //   double bad_visits = (bad_sites & visited_sites).CountOnes();
  //   double result = good_visits - bad_visits/2;
  //   return emp::max(0.0, result);
  // };
  // world.SetFitFun(fit_fun);

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

    // Keep the best individual.
    EliteSelect(world, ELITE_SIZE, ELITE_COPIES);

    std::cout << "  fitness[0] = " << world[0].GetScore()
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
