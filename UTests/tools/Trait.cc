#include <iostream>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Trait.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::TraitManager<int, double, std::vector<bool>, char, std::string> tm;
  std::cout << "# Types in Trait manager = " << tm.GetNumTypes() << std::endl;

  tm.AddTrait<int>("test_trait", "This is a test trait", 42);
  tm.AddTrait<std::string>("test2", "This is technically our second test trait.", "VALUE");
  tm.AddTrait<int>("test3", "And we need another int trait to test", 1000);

  std::cout << "Total num Traits = "        << tm.GetNumTraits() << std::endl
            << "Num int Traits = "    << tm.GetNumTraitsOfType<int>() << std::endl
            << "Num double Traits = " << tm.GetNumTraitsOfType<double>() << std::endl
            << "Num string Traits = " << tm.GetNumTraitsOfType<std::string>() << std::endl
    ;

  //emp::TraitSet trait_set(tm);

  return 0;
}
