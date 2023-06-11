/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file graph_utils.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/graph_utils.hpp"

TEST_CASE("Test graph_utils", "[datastructs]")
{
  emp::Random random;

  // build_graph_ring
  /// comment: this build graph ring doesn't seem to work right
  /// comment says: Construct a graph where all vertics are degree two and form a single ring.
  /// but often get outputs like this:
  /// 5 4
  /// 0 1
  /// 0 2
  /// 0 4
  /// 1 4
  /// where 3 isn't even included and it forms a ring but not where all vertices are degree two....
  emp::Graph gr = emp::build_graph_ring(5, random);
  REQUIRE(gr.GetSize() == 5);
  //REQUIRE(gr.GetEdgeCount() == 10);
  //gr.PrintSym();

  // build_graph_tree
  /// got output like:
  /// 5 3
  /// 0 1
  /// 0 2
  /// 0 3
  /// 4 isn't connected to the tree is that supposed to happen?
  /// is it supposed to be a binary tree?
  emp::Graph gt = emp::build_graph_tree(5, random);
  REQUIRE(gt.GetSize() == 5);
  //gt.PrintSym();

  // build_graph_random
  /// build graph tree is used in the connected version of build graph random,
  /// it's messing up the edge count, it assumes build graph tree has vertices - 1 edges
  /// when u can see the output above I got 5 v and 3 edges, so vertices - 2 edges.
  /// which explains why when I say 5 vertices and 7 edges it returns a graph like so:
  /// 5 6
  /// 0 1
  /// 0 4
  /// 1 3
  /// 1 4
  /// 2 4
  /// 3 4
  //grc.PrintSym();

  /// unconnected doesn't have this issue b/c it doesn't rely on build_graph_tree
  emp::Graph gru = build_graph_random(5, 7, random, false);
  REQUIRE(gru.GetSize() == 5);
  REQUIRE(gru.GetEdgeCount() == 14);

  // build_graph_clique_set - this one confuses me
  //emp::Graph gcs = build_graph_clique_set(1, 3, random);
  //gcs.PrintDirected();

  // build graph dag
  /// this doesn't really create DAGs right?
  /// if its supposed to be acyclic, test for that.
  emp::Graph gd = build_graph_dag(4, 3, random, true);
  REQUIRE(gd.GetSize() == 4);
  //REQUIRE(gd.GetEdgeCount() == 3); // edge count never seems to be accurate?


  double max_wght = 5.0;
  double min_wght = 1.0;

  // build weighted graph tree
  emp::WeightedGraph wgt = build_weighted_graph_tree(4, min_wght, max_wght, random);
  REQUIRE(wgt.GetSize() == 4);
  // ensure weights within range
  for(size_t i=0;i<wgt.GetSize();i++){
    for(size_t j=0;j<wgt.GetSize();j++){
      if(wgt.HasEdge(i,j)){
        double wght = wgt.GetWeight(i,j);
        REQUIRE((wght < max_wght));
        REQUIRE((wght >= min_wght));
      }
    }
  }

  // build weighted graph random
  emp::WeightedGraph wgr = build_weighted_graph_random(4, 4, min_wght, max_wght, random, true);
  REQUIRE(wgr.GetSize() == 4);
  // doesn't ensure edge count is 4?
  // ensure weights within range
  for(size_t i=0;i<wgr.GetSize();i++){
    for(size_t j=0;j<wgr.GetSize();j++){
      if(wgr.HasEdge(i,j)){
        double wght = wgr.GetWeight(i,j);
        REQUIRE((wght < max_wght));
        REQUIRE((wght >= min_wght));
      }
    }
  }

  // load graph sym from input stream
  std::stringstream ss;
  ss << "4 3\n0 1\n2 3\n0 3\n";
  emp::Graph load_gs = emp::load_graph_sym(ss);
  REQUIRE(load_gs.GetSize() == 4);
  REQUIRE(load_gs.GetEdgeCount() == 6);
  REQUIRE(load_gs.HasEdge(0,1));
  REQUIRE(load_gs.HasEdge(2,3));
  REQUIRE(load_gs.HasEdge(0,3));
  REQUIRE(!load_gs.HasEdge(1,2));

  // load graph table from input stream
  std::stringstream ss2;
  ss2 << "3 0 1 0 1 0 0 0 0 1";
  emp::Graph load_gt = emp::load_graph_table(ss2);
  REQUIRE(load_gt.GetSize() == 3);
  REQUIRE(load_gt.GetEdgeCount() == 3);
  REQUIRE(load_gt.HasEdgePair(0,1));
  REQUIRE(load_gt.HasEdge(2,2));
  REQUIRE(!load_gt.HasEdge(0,0));










}

// // TODO: add asserts
// emp::Random grand;
TEST_CASE("Test Graph utils", "[datastructs]")
{
  emp::Random random(1);
  // emp::Graph graph( emp::build_graph_tree(20, random) );
  // emp::Graph graph( emp::build_graph_random(20, 40, random) );
  emp::Graph graph( emp::build_graph_grid(5, 4, random) );

  // graph.PrintSym();
}
