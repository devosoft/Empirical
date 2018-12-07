#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/graph_utils.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test graph_utils", "[tools]")
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
	emp::Graph grc = build_graph_random(5, 7, random, true);
	//grc.PrintSym();
	
	/// unconnected doesn't have this issue b/c it doesn't rely on build_graph_tree
	emp::Graph gru = build_graph_random(5, 7, random, false);
	REQUIRE(gru.GetSize() == 5);
	REQUIRE(gru.GetEdgeCount() == 14);
	
	// build_graph_clique_set - this one confuses me
	//emp::Graph gcs = build_graph_clique_set(2, 2, random);
	//gcs.PrintSym();
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}