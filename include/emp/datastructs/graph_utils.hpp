/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file graph_utils.hpp
 *  @brief This file provides a number of tools for manipulating graphs.
 *  @note Status: BETA
 */

#ifndef EMP_DATASTRUCTS_GRAPH_UTILS_HPP_INCLUDE
#define EMP_DATASTRUCTS_GRAPH_UTILS_HPP_INCLUDE

#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../math/Random.hpp"
#include "../math/random_utils.hpp"

#include "Graph.hpp"
#include "vector_utils.hpp"

namespace emp {

  /// Take an existing graph, and build a new one that is isomorphic to it, but with randomized
  /// vertex IDs.
  Graph shuffle_graph(const Graph & in_graph, Random & random) {
    const size_t N = in_graph.GetSize();
    Graph out_graph(N);

    // Determine new vertex IDs
    emp::vector<size_t> v_map = BuildRange<size_t>(0, N);
    Shuffle(random, v_map);

    // Put the mapped edges into the new graph.
    for (size_t from = 0; from < N; from++) {
      for (size_t to = 0; to < N; to++) {
        if (in_graph.HasEdge(from, to)) {
          out_graph.AddEdge( v_map[from], v_map[to] );
        }
      }
    }

    return out_graph;
  }

  /// Construct a graph where all vertics are degree two and form a single ring.
  Graph build_graph_ring(size_t v_count, Random & random) {
    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t i = 1; i < v_count; i++) {
      const size_t from = v_map[i];
      const size_t to = v_map[i-1];
      graph.AddEdgePair(from, to);
    }

    graph.AddEdgePair(v_map[0], v_map[v_count-1]);

    return graph;
  }

  /// Construct a random tree graph (new vertices are repeatedly attached to a random position
  /// in a tree as it is constructed.)
  Graph build_graph_tree(size_t v_count, Random & random) {
    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t i = 1; i < v_count; i++) {
      const size_t from = v_map[i];
      const size_t to = v_map[random.GetUInt(i)];
      graph.AddEdgePair(from, to);
    }

    return graph;
  }

  /// Construct a random, graph with the specified number of vertices and edges.  If connected is
  /// set, start by building a tree.  Then connect random (unconnected) pairs of vertices until
  /// the proper number of edges are included.
  Graph build_graph_random(size_t v_count, size_t e_count, Random & random, bool connected=true)
  {
    const size_t max_edges = v_count * (v_count-1) / 2;
    (void) max_edges;

    emp_assert(v_count >= 2 && e_count > 0); // We need at least two vertices to support an edge.
    emp_assert(e_count <= max_edges, e_count, max_edges); // Shouldn't have more edges than can fit!

    Graph graph(v_count);
    size_t e_cur = 0;           // How many edges have we added?

    // If the graph should be connected, start by building a tree.
    if (connected) {
      emp_assert(e_count >= v_count - 1);  // We need enough edges to build a connected graph.
      graph = build_graph_tree(v_count, random);
      e_cur = v_count - 1;
    }

    // @CAO -- we should do something better if we are filling in most of the edges.

    while (e_cur < e_count) {
      const size_t from = random.GetUInt(v_count);
      const size_t to = random.GetUInt(v_count);

      if (from == to || graph.HasEdge(from,to)) continue;

      graph.AddEdgePair(from, to);
      ++e_cur;
    }

    return graph;
  }

  /// Construct a graph with width x height vertices setup into a grid structure.
  Graph build_graph_grid(size_t width, size_t height, Random & random, double prob_use=1.0) {
    emp_assert(width > 0 && height > 0);

    const size_t v_count = width * height;
    // const size_t e_count = (width-1)*height + width*(height-1);

    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t x=0; x < width; ++x) {
      for (size_t y=0; y < height; ++y) {
        const size_t from = y*width + x;
        if (x != (width-1) && random.P(prob_use)) {
          graph.AddEdgePair(v_map[from], v_map[from+1]);      // Horizontal
        }
        if (y != (height-1) && random.P(prob_use)) {
          graph.AddEdgePair(v_map[from], v_map[from+width]);  // Vertical
        }
      }
    }

    return graph;
  }

  /// Build a set of cliques (such that one member of each can be part of an independent set)
  /// and then links them together
  Graph build_graph_clique_set(size_t clique_size, size_t clique_count, Random & random,
                               double extra_prob=0.5) {
    emp_assert(clique_size > 0 && clique_count > 0);

    const size_t v_count = clique_size * clique_count;
    Graph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    // Fill out all of the edges within a clique
    for (size_t start_id = 0; start_id < v_count; start_id += clique_size) {
      const size_t end_id = start_id + clique_size;
      for (size_t node1 = start_id; node1 < end_id; node1++) {
        for (size_t node2 = node1+1; node2 < end_id; node2++) {
          graph.AddEdgePair(v_map[node1], v_map[node2]);
        }
      }
    }

    // Add on extra edges.
    for (size_t start1 = 0; start1 < v_count; start1 += clique_size) {
      const size_t end1 = start1 + clique_size;
      for (size_t start2 = start1+clique_size; start2 < v_count; start2 += clique_size) {
        const size_t end2 = start2 + clique_size;
        for (size_t node1 = start1; node1 < end1; node1++) {
          for (size_t node2 = start2; node2< end2; node2++) {
            if (node1 == start1 && node2 == start2) continue;  // Both part of IS.
            if (random.P(extra_prob)) graph.AddEdgePair(v_map[node1], v_map[node2]);
          }
        }
      }
    }

    return graph;
  }


  /// Construct a random, graph with the specified number of vertices and edges.  If connected is
  /// set, start by building a tree.  Then connect random (unconnected) pairs of vertices until
  /// the proper number of edges are included.
  Graph build_graph_dag(size_t v_count, size_t e_count, Random & random, bool connected=true)
  {
    const size_t max_edges = v_count * (v_count-1) / 2;
    (void) max_edges;

    emp_assert(v_count >= 2 && e_count > 0); // We need at least two vertices to support an edge.
    emp_assert(e_count <= max_edges);        // Shouldn't have more edges than can fit!

    Graph graph(v_count);                    //
    size_t e_cur = 0;                        // How many edges have we added?

    // If the graph should be connected, start by building a tree.
    if (connected) {
      emp_assert(e_count >= v_count - 1);    // We need enough edges to build a connected graph.

      // Determine order to connect in new vertices.
      emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
      Shuffle(random, v_map);

      // Connect in each vertex to the tree.
      for (size_t i = 1; i < v_count; i++) {
        size_t from = v_map[i];                // Pick the next node in the shuffle.
        size_t to = v_map[random.GetUInt(i)];  // Pick node already in the tree.
        if (from > to) std::swap(from, to);    // Make sure lower number is first.
        graph.AddEdge(from, to);
      }
      e_cur = v_count - 1;
    }

    // @CAO -- we should do something better if we are filling in most of the edges.

    while (e_cur < e_count) {
      size_t from = random.GetUInt(v_count);
      size_t to = random.GetUInt(v_count);

      if (from == to || graph.HasEdge(from,to)) continue;
      if (from > to) std::swap(from, to); // Make sure lower number is first.

      graph.AddEdge(from, to);
      ++e_cur;
    }

    // Make sure the edge ID numbers that we return are not all in order.
    return shuffle_graph(graph, random);
  }

  /// Construct a random WEIGHTED tree graph (new vertices are repeatedly attached to a random
  /// position in a tree as it is constructed.)
  WeightedGraph build_weighted_graph_tree(size_t v_count, size_t min_weight, size_t max_weight,
                                          Random & random) {
    WeightedGraph graph(v_count);

    emp::vector<size_t> v_map = BuildRange<size_t>(0, v_count);
    Shuffle(random, v_map);

    for (size_t i = 1; i < v_count; i++) {
      const size_t from = v_map[i];
      const size_t to = v_map[random.GetUInt(i)];
      const size_t weight = (size_t) random.GetDouble(min_weight, max_weight);
      graph.AddEdgePair(from, to, weight);
    }

    return graph;
  }

  /// Construct a random, WEIGHTED graph with the specified number of vertices, edges, and range
  /// of edge weights.  If connected is set, start by building a tree.  Then connect random
  /// (unconnected) pairs of vertices until the proper number of edges are included.
  WeightedGraph build_weighted_graph_random(size_t v_count, size_t e_count,
                                            size_t min_weight, size_t max_weight,
                                            Random & random, bool connected=true)
  {
    const size_t max_edges = v_count * (v_count-1) / 2;
    (void) max_edges;

    emp_assert(v_count >= 2 && e_count > 0); // We need at least two vertices to support an edge.
    emp_assert(e_count <= max_edges, e_count, max_edges); // Shouldn't have more edges than can fit!

    WeightedGraph graph(v_count);
    size_t e_cur = 0;           // How many edges have we added?

    // If the graph should be connected, start by building a tree.
    if (connected) {
      emp_assert(e_count >= v_count - 1);  // We need enough edges to build a connected graph.
      graph = build_weighted_graph_tree(v_count, min_weight, max_weight, random);
      e_cur = v_count - 1;
    }

    // @CAO -- we should do something better if we are filling in most of the edges.

    while (e_cur < e_count) {
      const size_t from = random.GetUInt(v_count);
      const size_t to = random.GetUInt(v_count);

      if (from == to || graph.HasEdge(from,to)) continue;

      graph.AddEdgePair(from, to, (size_t) random.GetDouble(min_weight, max_weight));
      ++e_cur;
    }

    return graph;
  }





  /// Helper function for loading symmetric graphs from an input stream.
  /// sub1 indicates that verticies are numbered 1 to N instead of 0 to N-1.
  // @CAO Need some error checking here...
  Graph load_graph_sym(std::istream & is, bool sub1=false) {
    size_t n_vert, n_edge;
    is >> n_vert >> n_edge;

    Graph out_graph(n_vert);
    size_t from, to;
    for (size_t i = 0; i < n_edge; i++) {
      is >> from >> to;
      if (sub1) { from--; to--; }
      out_graph.AddEdgePair(from, to);
    }

    return out_graph;
  }

  /// Load a graph with a specified filename.
  Graph load_graph_sym(std::string filename, bool sub1=false) {
    std::ifstream ifile(filename);
    return load_graph_sym(ifile, sub1);
  }

  /// Load a graph from a connection matrix.
  /// Format: Number of vertices followed by v^2 0's or 1's
  /// Example: "3 0 1 0 1 0 0 0 0 1"
  Graph load_graph_table(std::istream & is) {
    size_t n_vert;
    is >> n_vert;

    Graph out_graph(n_vert);
    size_t val;
    for (size_t i = 0; i < n_vert; i++) {
      for (size_t j = 0; j < n_vert; j++) {
        is >> val;
        if (val) out_graph.AddEdge(i, j);
      }
    }

    return out_graph;
  }

  /// Load a graph from a connection matrix in a file by the specified name.
  Graph load_graph_table(std::string filename) {
    std::ifstream ifile(filename);
    return load_graph_table(ifile);
  }
}

#endif // #ifndef EMP_DATASTRUCTS_GRAPH_UTILS_HPP_INCLUDE
