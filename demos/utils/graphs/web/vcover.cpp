#include <iostream>
#include <map>
#include <string>

#include "emp/base/assert.hpp"
#include "emp/config/command_line.hpp"
#include "emp/datastructs/Graph.hpp"
#include "emp/datastructs/graph_utils.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/tools/SolveState.hpp"

#include <emscripten.h>



bool verbose;   // Should we print extra information about solving progress?
bool debug;     // Should we print extra information to help diagnose problems?

emp::Graph graph;           // The graph we are trying to solve.
int best_count;             // The size of the best solution found (for quick comparison)
emp::BitVector best_nodes;  // The nodes included in the best solution found.

// Test the quality of a prospective solution -- return true if it's valid, false otherwise.
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

// std::vector<emp::BitVector> in_vector;   // Which values have been included at each level?
// std::vector<emp::BitVector> test_vector; // Which values still have to be tested at each level?

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

  emp::SolveState state(in_state);                     // @CAO Cache these!

  // Scan the remaining nodes.
  int test_id = -1;
  emp::BitVector degree_mask = ~(state.GetInVector());       // Count edges only to not-yet-included nodes.
  while ((test_id = state.GetNextUnk(test_id)) != -1) {
    const int cur_degree = graph.GetMaskedDegree(test_id, degree_mask);
    // If the test degree is zero, exclude it!
    if (cur_degree == 0) { state.Exclude(test_id); continue; }
    if (cur_degree == 1) {
      state.Exclude(test_id);                       // Exclude this degree-one node.
      state.IncludeSet(graph.GetEdgeSet(test_id));  // Include the node it is connected to.
      degree_mask = ~(state.GetInVector());         // Update the mask!
    }
  }

  // If there are no nodes left to test, let the early solver deal with it.
  if (state.IsFinal()) { Solve(state, depth+1); return; }

  // Now that we've simplified and know we have more to do, find the max degree
  int max_degree = -1;
  int max_id = -1;
  test_id = -1;
  while ((test_id = state.GetNextUnk(test_id)) != -1) {
    const int cur_degree = graph.GetMaskedDegree(test_id, degree_mask);
    if (cur_degree > max_degree) { max_degree = cur_degree; max_id = test_id; }
  }

  // Continue recursion... First include max degree remaining...
  state.Include(max_id);
  Solve(state, depth+1);

  // Then exclude
  state.ForceExclude(max_id);
  state.IncludeSet(graph.GetEdgeSet(max_id));
  Solve(state, depth+1);
}



extern "C" int empLoadString(char * _string)
{
  std::stringstream input_stream;
  input_stream << _string;

  graph = emp::load_graph_sym(input_stream);

  best_count = graph.GetSize();
  best_nodes.Resize(best_count);
  best_nodes.SetAll();

  FindInitBound();
  Solve(emp::SolveState(graph.GetSize()));

  EM_ASM_ARGS({
      var out_obj = document.getElementById("container");
      out_obj.innerHTML = "Result = " + $0;
  }, best_count);


  return 0;
}
