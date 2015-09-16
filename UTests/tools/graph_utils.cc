#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Graph.h"
#include "../../tools/graph_utils.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Random random;
  // emp::Graph graph( emp::build_graph_tree(20, random) );
  // emp::Graph graph( emp::build_graph_random(20, 40, random) );
  emp::Graph graph( emp::build_graph_grid(5, 4, random) );

  graph.PrintSym();
  
  if (verbose) std::cout << "Done!" << std::endl;
}
