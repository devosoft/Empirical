// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "base/vector.h"
#include "config/command_line.h"

#include "../ArmWorld.h"

int main(int argc, char* argv[])
{
  constexpr size_t MAX_GENS = 100;
  constexpr size_t POP_SIZE = 100;

  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  std::cout << "Hello World!" << std::endl;

  emp::Random random;
  ArmWorld world(random);

  // Setup the print function to indicate if a function exists.
  std::function<void(ArmOrg&,std::ostream &)> print_fun = [](ArmOrg & val, std::ostream & os) {
    std::string out_str = "!"; // emp::to_string(val);
    // while (out_str.size() < 4) out_str = emp::to_string('!');
    os << out_str;
  };
  world.SetPrintFun(print_fun);


  // Loop through updates
  for (size_t ud = 0; ud < MAX_GENS; ud++) {
    // Print current state.
    std::cout << "UD " << ud << std::endl;
    
    //  << ": " << world[0].ToString()
    //           << "  pop_size = " << world.GetSize()
    //           << std::endl;
    world.PrintGrid();

    // Keep the best individual.
    emp::EliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(world, 5, POP_SIZE-1);
    world.Update();
  }

  std::cout << "\nFINAL\n";
  world.PrintGrid();

}
