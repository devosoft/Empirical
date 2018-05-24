#include <iostream>

#include "base/vector.h"
#include "config/command_line.h"

#include "../AagosOrg.h"
#include "../AagosWorld.h"

int main(int argc, char* argv[])
{
  constexpr size_t NUM_BITS = 64;
  constexpr size_t NUM_GENES = 64;
  constexpr size_t GENE_SIZE = 8;

  AagosWorld world(NUM_BITS, NUM_GENES, GENE_SIZE);

  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  std::cout << "Hello World!" << std::endl;
}
