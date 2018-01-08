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

constexpr size_t UPDATES = 1000;       // How many generations to run?

int main()
{
  emp::Random random;
  SGWorld world(random, "AvidaWorld");

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Progress output...
    std::cout << "Update " << ud;

    world.RunUpdate();

    // Periodically, provide the status of the best organism.
    world.GetOrg(0).ResetHardware();
    world.GetOrg(0).Process(world.CPU_TIME);
    if (ud % 10 == 0) {
      std::cout << std::endl;
      world[0].GetSGStatus().PrintHistory(world.GetStateGrid());
    }
    std::cout << "  fitness[0] = " << world.CalcFitnessID(0) << std::endl;
  }

  std::cout << "Final Fitness: " << world.CalcFitnessID(0) << std::endl;
  world.ResetHardware();
  world.Process(world.CPU_TIME);
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
