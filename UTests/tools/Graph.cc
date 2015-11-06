// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Graph.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Graph graph(20);

  emp_assert(graph.GetSize() == 20);

  graph.AddEdgePair(0, 1);
  graph.AddEdgePair(0, 2);
  graph.AddEdgePair(0, 3);
  
  if (verbose) {
    std::cout << "Graph size = " << graph.GetSize() << std::endl
              << "Edge count = " << graph.GetEdgeCount() << std::endl
              << "Degree[0] = " << graph.GetDegree(0) << std::endl;
  }
  
  if (verbose) std::cout << "Done!" << std::endl;
}
