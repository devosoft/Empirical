/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  Graph.h
 *  @brief A simple, fast class for managing verticies (nodes) and edges.
 *  @note Status: BETA
 */

#ifndef EMP_GRAPH_H
#define EMP_GRAPH_H

#include <iostream>

#include "../base/assert.h"
#include "../base/vector.h"

#include "BitVector.h"

namespace emp {

  /// A graph class that maintains a set of vertices (nodes) and edges (connecting pairs of nodes)
  class Graph {
  public:
    /// Information about nodes within a graph.
    class Node {
    private:
      BitVector edge_set; /// What other node IDs is this one connected to?
    public:
      Node(size_t num_nodes) : edge_set(num_nodes) { ; }
      Node(const Node & in_node) : edge_set(in_node.edge_set) { ; }
      ~Node() { ; }

      /// Set this node to have the same connections as another node.
      Node & operator=(const Node & in_node) { edge_set = in_node.edge_set; return *this; }

      /// Is this node connect to a specific other node?
      bool HasEdge(size_t to) const { return edge_set[to]; }

      /// Add a connection between this node and another.
      void AddEdge(size_t to) { edge_set.Set(to, true); }

      /// Remove the connection (if there is one) between this node and another one.
      void RemoveEdge(size_t to) { edge_set.Set(to, false); }

      /// Set whether a connection to another specific node should exist or not.
      void SetEdge(size_t to, bool val) { edge_set.Set(to, val); }

      /// Get a BitVector representing which nodes this one is connected to.
      const BitVector & GetEdgeSet() const { return edge_set; }

      /// Change the number of potential node connections that we are tracking.
      void Resize(size_t new_size) { edge_set.Resize(new_size); }

      /// Remove all edges from this node.
      void Clear() { edge_set.Clear(); }

      /// Identify how many other nodes this one is connected to.
      size_t GetDegree() const { return edge_set.CountOnes(); }

      /// Identify how many other nodes from a provided set (a BitVector) this one is connected to.
      size_t GetMaskedDegree(const BitVector & mask) const { return (mask & edge_set).CountOnes(); }
    };

  private:
    emp::vector<Node> nodes;  ///< Set of vertices in this graph.

  public:
    /// Construct a new graph with the specified number of nodes.
    Graph(size_t num_nodes=0) : nodes(num_nodes, num_nodes) { ; }

    Graph(const Graph &) = default;              ///< Copy constructor
    Graph(Graph &&) = default;                   ///< Move constructor
    ~Graph() { ; }

    Graph & operator=(const Graph &) = default;  ///< Copy operator
    Graph & operator=(Graph &&) = default;       ///< Move operator

    /// Get number of vertices in this graph.
    size_t GetSize() const { return nodes.size(); }

    /// Get the total number of edges in this graph.
    size_t GetEdgeCount() const {
      size_t edge_count = 0;
      for (size_t i = 0; i < nodes.size(); i++) edge_count += nodes[i].GetDegree();
      return edge_count;
    }

    /// Change the number of vertices in this graph.
    void Resize(size_t new_size) {
      nodes.resize(new_size, new_size);
      for (auto & node : nodes) {
	      node.Resize(new_size);
	      node.Clear();
      }
    }

    /// Get the set of nodes that a specified node is connected to.
    const BitVector & GetEdgeSet(size_t id) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetEdgeSet();
    }

    /// Get the degree of a specified node.
    size_t GetDegree(size_t id) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetDegree();
    }

    /// Get how many of a set of nodes that a specified node is connected to.
    size_t GetMaskedDegree(size_t id, const BitVector & mask) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetMaskedDegree(mask);
    }

    /// Determine if a specific edge is included in this graph.
    bool HasEdge(size_t from, size_t to) const {
      emp_assert(from < nodes.size() && to < nodes.size());
      return nodes[from].HasEdge(to);
    }

    /// Add a specified edge into this graph.
    void AddEdge(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].AddEdge(to);
    }

    /// Remove a specified edge from this graph
    void RemoveEdge(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].RemoveEdge(to);
    }

    /// Set the status of a specified edge as to whether or not it should be in the graph.
    void SetEdge(size_t from, size_t to, bool val) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].SetEdge(to, val);
    }

    /// Determine if edges exist in both directions between a pair of vertices.
    bool HasEdgePair(size_t from, size_t to) const {
      emp_assert(from < nodes.size() && to < nodes.size());
      return nodes[from].HasEdge(to) && nodes[to].HasEdge(from);
    }

    /// Add a pair of edges between two vertieces (in both directions)
    void AddEdgePair(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].AddEdge(to);
      nodes[to].AddEdge(from);
    }

    /// Remove edges in both directions between a pair of vertices.
    void RemoveEdgePair(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].RemoveEdge(to);
      nodes[to].RemoveEdge(from);
    }

    /// Set the status as to whether a pair of edges (in both direction) exist.
    void SetEdgePairs(size_t from, size_t to, bool val) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].SetEdge(to, val);
      nodes[to].SetEdge(from, val);
    }

    /// Print the graph to the provided output stream (defaulting to standard out)
    void PrintSym(std::ostream & os=std::cout) {
      os << GetSize() << " " << (GetEdgeCount()/2) << std::endl;
      for (size_t from = 0; from < nodes.size(); from++) {
        for (size_t to=from+1; to < nodes.size(); to++) {
          if (HasEdge(from, to) == false) continue;
          os << from << " " << to << std::endl;
        }
      }
    }

  };

}

#endif
