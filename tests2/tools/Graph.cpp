/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file Graph.cc
 *  @brief Unit tests for Graph and WeightedGraph
 */

#include "../../source/tools/Graph.h"

#include "../unit_tests.h"


void emp_test_main()
{
  emp::WeightedGraph graph(10);
  emp::WeightedGraph graph2(graph);

  EMP_TEST_VALUE(graph.GetSize(), 10);
  EMP_TEST_VALUE(graph2.GetSize(), 10);
  EMP_TEST_VALUE(graph.GetEdgeCount(), 0);
  EMP_TEST_VALUE(graph2.GetEdgeCount(), 0);

  graph.AddEdge(0, 3, 10.0);
  graph.AddEdge(3, 1, 20.0);
  graph.AddEdge(1, 9, 30.0);

  // Make sure new edges are in graph, but not graph2
  EMP_TEST_VALUE(graph.GetEdgeCount(), 3);
  EMP_TEST_VALUE(graph2.GetEdgeCount(), 0);
  
  graph = graph2;

  // Both graphs should be back to zero edges
  EMP_TEST_VALUE(graph.GetEdgeCount(), 0);
  EMP_TEST_VALUE(graph2.GetEdgeCount(), 0);

  emp::WeightedGraph graph3(1);
}
