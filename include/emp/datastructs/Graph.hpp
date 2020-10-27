/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  Graph.hpp
 *  @brief A simple, fast class for managing verticies (nodes) and edges.
 *  @note Status: BETA
 */

#ifndef EMP_GRAPH_H
#define EMP_GRAPH_H

#include <iostream>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../bits/BitVector.hpp"

namespace emp {

  /// A graph class that maintains a set of vertices (nodes) and edges (connecting pairs of nodes)
  class Graph {
  public:
    /// Information about nodes within a graph.
    class Node {
    private:
      BitVector edge_set; /// What other node IDs is this one connected to?
      std::string label;
    public:
      Node(size_t num_nodes) : edge_set(num_nodes), label("") { ; }
      Node(const Node & in_node) : edge_set(in_node.edge_set), label(in_node.label) { ; }
      ~Node() { ; }

      /// Set this node to have the same connections as another node.
      Node & operator=(const Node & in_node) { edge_set = in_node.edge_set; return *this; }

      /// Is this node connect to a specific other node?
      bool HasEdge(size_t to) const { return edge_set[to]; }

      /// Add a connection between this node and another.
      void AddEdge(size_t to) { edge_set.Set(to, true); }

      /// Add a full set of connections from this node to others.
      void AddEdgeSet(BitVector in_set) { edge_set |= in_set; }

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

      void SetLabel(std::string lab) {
        label = lab;
      }

      std::string GetLabel() {
        return label;
      }

    };

  protected:
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

    Node GetNode(int i) {return nodes[i];}
    emp::vector<Node> GetNodes(){return nodes;}
    
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
    /// For directed graphs, this is the out-degree
    size_t GetDegree(size_t id) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetDegree();
    }

    /// Get the in-degree (number of incoming edges)
    /// of the node @param id. 
    /// This should only be used for directed graphs (for
    /// undirected graphs, GetDegree() is equivalent and faster) 
    size_t GetInDegree(size_t id) const {
      size_t count = 0;
      for (auto & node : nodes) {
        // Node is allowed to to have edge to itself so it's
        // okay that we don't exclude it
        if (node.HasEdge(id)) {
          count++;
        }
      }
      return count;
    }

    /// Get how many of a set of nodes that a specified node is connected to.
    size_t GetMaskedDegree(size_t id, const BitVector & mask) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetMaskedDegree(mask);
    }

    /// Set label of node @param id
    void SetLabel(size_t id, std::string lab) {
      nodes[id].SetLabel(lab);
    }

    /// Get label of node @param id
    std::string GetLabel(size_t id) {
      return nodes[id].GetLabel();
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

    /// Merge a second graph into this one.
    void Merge(const Graph & in_graph) {
      const size_t start_size = nodes.size();
      const size_t new_size = start_size + in_graph.GetSize();
      nodes.resize(new_size, new_size);
      for (auto & node : nodes) {
	      node.Resize(new_size);
      }

      for (size_t i = 0; i < in_graph.GetSize(); i++) {
        BitVector edge_set = in_graph.nodes[i].GetEdgeSet();
        edge_set.Resize(new_size);
        edge_set <<= start_size;
        nodes[start_size + i].AddEdgeSet(edge_set);
      }
    }

    /// Print a symmetric graph to the provided output stream (defaulting to standard out)
    void PrintSym(std::ostream & os=std::cout) {
      os << GetSize() << " " << (GetEdgeCount()/2) << std::endl;
      for (size_t from = 0; from < nodes.size(); from++) {
        for (size_t to=from+1; to < nodes.size(); to++) {
          if (HasEdge(from, to) == false) continue;
	        emp_assert(HasEdge(to, from));              // This must be a symmetric graph!
          os << from << " " << to << std::endl;
        }
      }
    }

    /// Print a directed graph to the provided output stream (defaulting to standard out)
    void PrintDirected(std::ostream & os=std::cout) {
      os << GetSize() << " " << GetEdgeCount() << std::endl;
      for (size_t from = 0; from < nodes.size(); from++) {
        for (size_t to = 0; to < nodes.size(); to++) {
          if (HasEdge(from, to) == false) continue;
          os << from << " " << to << std::endl;
        }
      }
    }

  };

  class WeightedGraph : public Graph {
  protected:
    emp::vector< emp::vector< double > > weights;

  public:
    WeightedGraph(size_t num_nodes=0) : Graph(num_nodes), weights(num_nodes) {
      for (auto & row : weights) row.resize(num_nodes, 0.0);
    }

    WeightedGraph(const WeightedGraph &) = default;              ///< Copy constructor
    WeightedGraph(WeightedGraph &&) = default;                   ///< Move constructor
    ~WeightedGraph() { ; }

    WeightedGraph & operator=(const WeightedGraph &) = default;  ///< Copy operator
    WeightedGraph & operator=(WeightedGraph &&) = default;       ///< Move operator

    void Resize(size_t new_size) {
      Graph::Resize(new_size);
      weights.resize(new_size);
      for (auto & row : weights) row.resize(new_size,0.0);
    }

    /// Determine weight of a specific edge in this graph.
    double GetWeight(size_t from, size_t to) const {
      emp_assert(from < nodes.size() && to < nodes.size());
      return weights[from][to];
    }

    /// When Adding an edge, must also provide a weight.
    void AddEdge(size_t from, size_t to, double weight) {
      Graph::AddEdge(from, to);
      weights[from][to] = weight;
    }

    /// When Adding an edge pair, must also provide a weight.
    void AddEdgePair(size_t from, size_t to, double weight) {
      Graph::AddEdgePair(from, to);
      weights[from][to] = weight;
      weights[to][from] = weight;
    }

    /// Merge two WeightedGraphs into one
    void Merge(const WeightedGraph & in_graph) {
      const size_t start_size = nodes.size();
      Graph::Merge(in_graph);
      weights.resize(nodes.size());
      for (auto & row : weights) row.resize(nodes.size(), 0.0);

      // Move the weights over.
      for (size_t i = 0; i < in_graph.GetSize(); i++) {
        for (size_t j = 0; j < in_graph.GetSize(); j++) {
          weights[i+start_size][j+start_size] = in_graph.weights[i][j];
        }
      }
    }

    /// Print a symmetric graph to the provided output stream (defaulting to standard out)
    void PrintSym(std::ostream & os=std::cout) {
      os << GetSize() << " " << (GetEdgeCount()/2) << std::endl;
      for (size_t from = 0; from < nodes.size(); from++) {
        for (size_t to=from+1; to < nodes.size(); to++) {
          if (HasEdge(from, to) == false) continue;
	        emp_assert(HasEdge(to, from));              // This must be a symmetric graph!
          os << from << " " << to << " " << weights[from][to] << std::endl;
        }
      }
    }

    /// Print a directed graph to the provided output stream (defaulting to standard out)
    void PrintDirected(std::ostream & os=std::cout) {
      os << GetSize() << " " << GetEdgeCount() << std::endl;
      for (size_t from = 0; from < nodes.size(); from++) {
        for (size_t to = 0; to < nodes.size(); to++) {
          if (HasEdge(from, to) == false) continue;
          os << from << " " << to << " " << weights[from][to] << std::endl;
        }
      }
    }

    emp::vector<emp::vector<double> > GetWeights(){return weights;}

  };
}

#endif
