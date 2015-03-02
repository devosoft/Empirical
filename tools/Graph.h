#ifndef EMP_GRAPH_H
#define EMP_GRAPH_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  A simple, fast graph class
//

#include <assert.h>
#include <iostream>
#include <vector>

#include "BitVector.h"

namespace emp {

  class Graph {
  public:
    class Node {
    private:
      BitVector edge_set;
    public:
      Node(int num_nodes) : edge_set(num_nodes) { ; }
      Node(const Node & in_node) : edge_set(in_node.edge_set) { ; }
      ~Node() { ; }

      Node & operator=(const Node & in_node) { edge_set = in_node.edge_set; return *this; }

      bool HasEdge(int to) const { return edge_set[to]; }
      void AddEdge(int to) { edge_set.Set(to, true); }
      void RemoveEdge(int to) { edge_set.Set(to, false); }
      void SetEdge(int to, bool val) { edge_set.Set(to, val); }

      int GetDegree() const { return edge_set.CountOnes(); }
    };

  private:
    std::vector<Node> nodes;

  public:
    Graph(int num_nodes) : nodes(num_nodes, num_nodes) { ; }
    Graph(const Graph & in_graph) : nodes(in_graph.nodes) { ; }
    ~Graph() { ; }

    Graph & operator=(const Graph & in_graph) { nodes = in_graph.nodes; return *this; }

    int GetSize() const { return (int) nodes.size(); }
    int GetEdgeCount() const {
      int edge_count = 0;
      for (int i = 0; i < (int) nodes.size(); i++) edge_count += nodes[i].GetDegree();
      return edge_count;
    }

    int GetDegree(int id) const {
      assert(id >= 0 && id < (int) nodes.size());
      return nodes[id].GetDegree();
    }


    bool HasEdge(int from, int to) const {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int)nodes.size());
      return nodes[from].HasEdge(to);
    }
    void AddEdge(int from, int to) {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      nodes[from].AddEdge(to);
    }
    void RemoveEdge(int from, int to) {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      nodes[from].RemoveEdge(to);
    }
    void SetEdge(int from, int to, bool val) {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      nodes[from].SetEdge(to, val);
    }


    bool HasEdgePair(int from, int to) const {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      return nodes[from].HasEdge(to) && nodes[to].HasEdge(from);
    }
    void AddEdgePair(int from, int to) {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      nodes[from].AddEdge(to);
      nodes[to].AddEdge(from);
    }
    void RemoveEdgePair(int from, int to) {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      nodes[from].RemoveEdge(to);
      nodes[to].RemoveEdge(from);
    }
    void SetEdgePairs(int from, int to, bool val) {
      assert(from >= 0 && from < (int) nodes.size() && to >= 0 && to < (int) nodes.size());
      nodes[from].SetEdge(to, val);
      nodes[to].SetEdge(from, val);
    }


    void PrintSym(std::ostream & os=std::cout) {
      os << GetSize() << std::endl;
      os << GetSize() << " " << (GetEdgeCount()/2) << std::endl;
      for (int from = 0; from < (int) nodes.size(); from++) {
        for (int to=from+1; to < (int) nodes.size(); to++) {
          if (HasEdge(from, to) == false) continue;
          os << from << " " << to << std::endl;
        }
      }
    }

  };

};

#endif
