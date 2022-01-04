/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Graph.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/datastructs/Graph.hpp"

TEST_CASE("Test Graph", "[datastructs]")
{
  // Constructor
  emp::Graph graph(10);
  REQUIRE((graph.GetEdgeCount() == 0));

  // AddEdge HasEdge RemoveEdge
  REQUIRE(!graph.HasEdge(0,1));
  graph.AddEdge(0,1);
  graph.AddEdge(2,4);
  REQUIRE(graph.HasEdge(0,1));
  REQUIRE(graph.HasEdge(2,4));
  REQUIRE((graph.GetEdgeCount() == 2));
  graph.RemoveEdge(2,4);
  REQUIRE(!graph.HasEdge(2,4));
  REQUIRE((graph.GetEdgeCount() == 1));

  // Labels
  graph.SetLabel(1, "node 1");
  REQUIRE(graph.GetLabel(1) == "node 1");

  // Degree
  REQUIRE(graph.GetInDegree(1) == 1);
  REQUIRE(graph.GetInDegree(0) == 0);
  REQUIRE(graph.GetDegree(0) == 1);

  // Getters
  emp::Graph::Node n = graph.GetNode(1);
  REQUIRE(n.GetLabel() == "node 1");
  emp::vector<emp::Graph::Node> nodes = graph.GetNodes();
  REQUIRE(nodes.size() == 10);
  REQUIRE(nodes[1].GetLabel() == "node 1");

  // Assignment
  emp::Graph g2 = graph;
  REQUIRE((g2.GetEdgeCount() == 1));
  REQUIRE(g2.HasEdge(0,1));

  // SetEdge
  g2.SetEdge(0,1,false);
  REQUIRE(!g2.HasEdge(0,1));
  g2.SetEdge(4,3,true);
  REQUIRE(g2.HasEdge(4,3));

  // GetDegree
  graph.AddEdge(0,3);
  graph.AddEdge(0,6);
  REQUIRE((graph.GetDegree(0) == 3));
  emp::BitVector bit_v(10);
  REQUIRE(graph.GetMaskedDegree(0, bit_v) == 0);
  bit_v.Set(3);
  REQUIRE(graph.GetMaskedDegree(0, bit_v) == 1);
  bit_v.Set(6);
  REQUIRE(graph.GetMaskedDegree(0, bit_v) == 2);

  // GetEdgeSet
  emp::BitVector bv = graph.GetEdgeSet(0);
  REQUIRE(!bv[0]);
  REQUIRE(bv[1]);
  REQUIRE(bv[3]);
  REQUIRE(bv[6]);

  // Resize
  graph.Resize(12); // clears graph
  graph.AddEdge(9,11);
  REQUIRE((graph.GetSize() == 12));
  REQUIRE((graph.GetDegree(9) == 1));

  // HasEdgePair AddEdgePair RemoveEdgePair
  graph.AddEdgePair(8,9);
  REQUIRE(graph.HasEdgePair(8,9));
  REQUIRE(graph.HasEdgePair(9,8));
  graph.AddEdge(0,3);
  graph.AddEdge(3,0);
  REQUIRE(graph.HasEdgePair(0,3));
  REQUIRE(graph.HasEdgePair(3,0));
  graph.RemoveEdgePair(8,9);
  REQUIRE(!graph.HasEdgePair(9,8));

  // SetEdgePairs
  graph.SetEdgePairs(0,3,false);
  REQUIRE(!graph.HasEdgePair(0,3));
  graph.SetEdgePairs(6,2,true);
  REQUIRE(graph.HasEdgePair(2,6));

  // Merge
  g2.Resize(4);
  g2.AddEdge(0,1);
  graph.Merge(g2);
  REQUIRE(graph.HasEdge(12,13));

  // Print
  std::stringstream ss;
  g2.AddEdge(1,0);
  REQUIRE(g2.HasEdgePair(0,1));
  g2.PrintSym(ss);
  REQUIRE((ss.str() == "4 1\n0 1\n"));
  ss.str(std::string()); // clearing ss
  graph.PrintDirected(ss);
  REQUIRE((ss.str() == "16 4\n2 6\n6 2\n9 11\n12 13\n"));
  ss.str(std::string()); // clearing ss

  /**
  *  Weighted Graph
  */
  // Constructor
  emp::WeightedGraph wgraph(5);
  REQUIRE((wgraph.GetEdgeCount() == 0));
  REQUIRE((wgraph.GetSize() == 5));

  // Resize
  wgraph.Resize(10);
  REQUIRE((wgraph.GetSize() == 10));

  // AddEdge GetWeight
  wgraph.AddEdge(0,1,3.2);
  REQUIRE(wgraph.HasEdge(0,1));
  REQUIRE((wgraph.GetWeight(0,1) == 3.2));
  REQUIRE((wgraph.GetDegree(0) == 1));

  emp::vector<emp::vector<double> > weights = wgraph.GetWeights();
  REQUIRE(weights[0][1] == 3.2);

  // AddEdgePair
  wgraph.AddEdgePair(3, 2, 1.5);
  REQUIRE(wgraph.HasEdgePair(3,2));
  REQUIRE(wgraph.HasEdge(3,2));
  REQUIRE(wgraph.HasEdge(2,3));
  REQUIRE((wgraph.GetWeight(3,2) == 1.5));
  REQUIRE((wgraph.GetWeight(2,3) == 1.5));

  // Merge
  emp::WeightedGraph wgraph2(5);
  wgraph2.AddEdge(3, 4, 15.1);
  wgraph.Merge(wgraph2);
  REQUIRE(wgraph.HasEdge(13,14));
  REQUIRE((wgraph.GetWeight(13,14) == 15.1));

  // Print
  /// comment: printsym in weighted graph doesn't check if the weights are the same
  wgraph2.AddEdge(4, 3, 9.5);
  wgraph2.PrintSym(ss);
  REQUIRE((ss.str() == "5 1\n3 4 15.1\n"));
  ss.str(std::string()); // clearing ss
  wgraph.PrintDirected(ss);
  REQUIRE((ss.str() == "15 4\n0 1 3.2\n2 3 1.5\n3 2 1.5\n13 14 15.1\n"));
  ss.str(std::string()); // clearing ss
}

// TODO: add moar asserts
TEST_CASE("Test graph", "[datastructs]")
{

  emp::Graph graph(20);

  REQUIRE(graph.GetSize() == 20);

  graph.AddEdgePair(0, 1);
  graph.AddEdgePair(0, 2);
  graph.AddEdgePair(0, 3);

}
