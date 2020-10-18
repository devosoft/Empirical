//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Determine the minimum vertex cover for a graph provided on standard input.

#include <iostream>
#include <map>

#include "../../../source/base/assert.h"
#include "../../../source/base/vector.h"
#include "../../../source/config/command_line.h"
#include "../../../source/tools/Graph.h"
#include "../../../source/tools/graph_utils.h"
#include "../../../source/tools/string_utils.h"
#include "../../../source/tools/SolveState.h"

bool verbose;   // Should we print extra information about solving progress?
bool debug;     // Should we print extra information to help diagnose problems?

emp::Graph graph;           // The graph we are trying to solve.
int best_count;             // The size of the best solution found (for quick comparison)
emp::BitVector best_nodes;  // The nodes included in the best solution found.

// Test the legality of a prospective solution -- return true if it's valid, false otherwise.
bool TestSolution(const emp::BitVector & nodes_in) {
  // Every node must either be included or all connections must be to included nodes.
  const emp::BitVector off_nodes(~nodes_in);
  int num_tests = off_nodes.CountOnes();
  emp::BitVector test_nodes(off_nodes);
  while (num_tests--) {
    const int test_id = test_nodes.PopBit();
    // Since this node is off, it shouldn't be connected to any off nodes.
    if ((graph.GetEdgeSet(test_id) & off_nodes).Any()) return false;
  }

  // If we made it here, all of the nodes check out!
  return true;
}

bool TestSolution(const emp::SolveState & solution) {
  return TestSolution(solution.GetInVector());
}

void FindInitBound() {
  emp::BitVector node_mask(graph.GetSize(), true);  // IDs of which nodes need to be decided upon.

  while (true) {
    // Find node of max untagged degree.
    int max_degree = 0;
    int max_id = -1;
    for (int i = 0; i < graph.GetSize(); i++) {
      if (!node_mask[i]) continue; // If already in set, skip!
      int cur_degree = graph.GetMaskedDegree(i, node_mask);
      if (cur_degree > max_degree) { max_degree = cur_degree; max_id = i; }
    }

    if (max_degree == 0) break; // If no untagged nodes left, stop!

    node_mask[max_id] = false;
  }

  best_nodes = ~node_mask;
  best_count = best_nodes.CountOnes();

  if (verbose) std::cout << "Init size: " << best_count << std::endl;
}


void Solve(const emp::SolveState & in_state, int depth=0)
{
  if (debug) std::cout << "Solve(" << depth << ")" << std::endl;

  // Simple Bounds tests
  const int cur_count = in_state.CountIn();
  if (cur_count >= best_count) return;

  // If there are no nodes left to test, examine this answer.
  if (in_state.IsFinal()) {
    if (TestSolution(in_state) == false) return;  // Ignore illegal answers.
    best_count = cur_count;                       // This must be the best answer so far!
    best_nodes = in_state.GetInVector();
    if (verbose) std::cout << "New best: " << best_count << std::endl;
    return;
  }

  emp::SolveState state(in_state);                     // @CAO Still need to cache these!

  // Scan the remaining nodes.
  int test_id = -1;
  const emp::BitVector & degree_mask = state.GetUnkVector(); // Count edges only to other unknowns
  while ((test_id = state.GetNextUnk(test_id)) != -1) {
    const int cur_degree = graph.GetMaskedDegree(test_id, degree_mask);
    // If the test degree is zero, exclude it!
    if (cur_degree == 0) {
      state.Exclude(test_id);
    }
    else if (cur_degree == 1) {
      state.Exclude(test_id);                       // Exclude this degree-one node.
      state.IncludeSet(graph.GetEdgeSet(test_id));  // Include the node it is connected to.
      test_id = -1;  // Start looking at nodes from the beginning.
    }
  }

  // If there are no nodes left to test, let the early solver deal with it.
  if (state.IsFinal()) { Solve(state, depth+1); return; }

  // Now that we've simplified and know we have more to do, find the max degree
  int max_degree = -1;
  int max_id = -1;
  int total_degree = 0;
  test_id = -1;
  while ((test_id = state.GetNextUnk(test_id)) != -1) {
    const int cur_degree = graph.GetMaskedDegree(test_id, degree_mask);
    total_degree += cur_degree;
    if (cur_degree > max_degree) { max_degree = cur_degree; max_id = test_id; }
  }

  // Better bounds checking: How few nodes can we get away with adding to cover all of the edges
  const int min_added = (total_degree - 2) / (max_degree * 2) + 1;
  if (state.CountIn() + min_added >= best_count) return;


  // Continue recursion... First include max degree remaining...
  state.Include(max_id);
  Solve(state, depth+1);

  // Then exclude
  state.ForceExclude(max_id);
  state.IncludeSet(graph.GetEdgeSet(max_id));
  Solve(state, depth+1);
}

int main(int argc, char* argv[])
{
  // Use some emp tools to check which args were set on the command line.
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  verbose = emp::cl::use_arg(args, "-v");
  debug = emp::cl::use_arg(args, "-d");
  bool off_by_1 = emp::cl::use_arg(args, "-1");   // Are nodes numbered 1 to N instead of 0 to N-1?
  bool load_table = emp::cl::use_arg(args, "-t"); // Do we want to load the full adj matrix?

  graph = (load_table) ? emp::load_graph_table(std::cin) : emp::load_graph_sym(std::cin, off_by_1);

  // Initialize best count and best nodes to be the whole graph.
  best_count = graph.GetSize();
  best_nodes.Resize(best_count);
  best_nodes.SetAll();

  FindInitBound();                          // Use a heuristic to find an initial bound.
  Solve(emp::SolveState(graph.GetSize()));  // Recursively search all possible solutions.

  std::cout << best_count << std::endl;
  if (verbose) {
    std::cout << "[ ";
    auto best_node_ids = best_nodes.GetOnes();
    for (int i : best_node_ids) std::cout << i << " ";
    std::cout << "]" << std::endl;
  }
}
