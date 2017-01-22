//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A simple, fast class for managing verticies (nodes) and edges.

#ifndef EMP_GRAPH_H
#define EMP_GRAPH_H

#include <iostream>

#include "assert.h"
#include "BitVector.h"
#include "vector.h"

namespace emp {

  class Graph {
  public:
    class Node {
    private:
      BitVector edge_set;
    public:
      Node(size_t num_nodes) : edge_set(num_nodes) { ; }
      Node(const Node & in_node) : edge_set(in_node.edge_set) { ; }
      ~Node() { ; }

      Node & operator=(const Node & in_node) { edge_set = in_node.edge_set; return *this; }

      bool HasEdge(size_t to) const { return edge_set[to]; }
      void AddEdge(size_t to) { edge_set.Set(to, true); }
      void RemoveEdge(size_t to) { edge_set.Set(to, false); }
      void SetEdge(size_t to, bool val) { edge_set.Set(to, val); }
      const BitVector & GetEdgeSet() const { return edge_set; }

      size_t GetDegree() const { return edge_set.CountOnes(); }
      size_t GetMaskedDegree(const BitVector & mask) const { return (mask & edge_set).CountOnes(); }
    };

  private:
    emp::vector<Node> nodes;

  public:
    Graph(size_t num_nodes=0) : nodes(num_nodes, num_nodes) { ; }
    Graph(const Graph & in_graph) : nodes(in_graph.nodes) { ; }
    ~Graph() { ; }

    Graph & operator=(const Graph & in_graph) { nodes = in_graph.nodes; return *this; }

    size_t GetSize() const { return nodes.size(); }
    size_t GetEdgeCount() const {
      size_t edge_count = 0;
      for (size_t i = 0; i < nodes.size(); i++) edge_count += nodes[i].GetDegree();
      return edge_count;
    }

    const BitVector & GetEdgeSet(size_t id) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetEdgeSet();
    }
    size_t GetDegree(size_t id) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetDegree();
    }
    size_t GetMaskedDegree(size_t id, const BitVector & mask) const {
      emp_assert(id < nodes.size());
      return nodes[id].GetMaskedDegree(mask);
    }


    bool HasEdge(size_t from, size_t to) const {
      emp_assert(from < nodes.size() && to < nodes.size());
      return nodes[from].HasEdge(to);
    }
    void AddEdge(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].AddEdge(to);
    }
    void RemoveEdge(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].RemoveEdge(to);
    }
    void SetEdge(size_t from, size_t to, bool val) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].SetEdge(to, val);
    }


    bool HasEdgePair(size_t from, size_t to) const {
      emp_assert(from < nodes.size() && to < nodes.size());
      return nodes[from].HasEdge(to) && nodes[to].HasEdge(from);
    }
    void AddEdgePair(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].AddEdge(to);
      nodes[to].AddEdge(from);
    }
    void RemoveEdgePair(size_t from, size_t to) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].RemoveEdge(to);
      nodes[to].RemoveEdge(from);
    }
    void SetEdgePairs(size_t from, size_t to, bool val) {
      emp_assert(from < nodes.size() && to < nodes.size());
      nodes[from].SetEdge(to, val);
      nodes[to].SetEdge(from, val);
    }


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
