#pragma once

#include "../../../include/emp/base/array.hpp"
#include "../../../include/emp/base/vector.hpp"

#include "Graph.hpp"

class Solver {
private:
  static constexpr bool VERBOSE = false;
  static constexpr size_t MAX_BITS = 256;
  using graph_t = Graph<MAX_BITS>;

  graph_t start_graph;
  graph_t best_graph;
  size_t best_size = 10000000;
  bool solvable = true;

  emp::array<graph_t, MAX_BITS> graphs;

  void MarkSolution(const graph_t & graph) {
    if (graph.GetIncludeSize() < best_size) {
      best_graph = graph;
      best_size = graph.GetIncludeSize();
      std::cerr << "New Best: " << best_size << std::endl;
    }
  }

  void PrintSolveState(const graph_t & graph, size_t depth) const {
    for (size_t i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << "Solve(" << depth << "):"
              << " keys:" << graph.GetKeys()
              << " included:" << graph.GetIncluded()
              << " unknown:" << graph.GetUnknown()
              << " solved:" << graph.IsSolved()
              << std::endl;    
  }

public:
  Solver(const graph_t & graph) : start_graph(graph) { }

  void Debug() {
    PrintSolveState(start_graph, 0);
    start_graph.Include(0);
    PrintSolveState(start_graph, 0);
  }

  void Solve() {
    if (!start_graph.TestSolvable()) {
      solvable = false;
      return;
    }
    graphs[0] = start_graph;
    graphs[0].Setup();
    Solve(0);
  }

  void Solve(size_t depth) {
    graph_t & graph = graphs[depth];
    emp_debug("SOLVE: ", depth, " key=", graph.GetKeys(), " included=", graph.GetIncluded(), " unknown=", graph.GetUnknown());

    if constexpr (VERBOSE) PrintSolveState(graph, depth);

    graph.Optimize();

    if (graph.IsSolved()) { MarkSolution(graph); return; }            // Solved!
    if (graph.TestOneNodeSolution()) { MarkSolution(graph); return; } // Solved with one new node.
    if (graph.TestTwoNodeSolution()) { MarkSolution(graph); return; } // Solved with two new nodes.
    if (graph.GetIncludeSize()+3 >= best_size) return;                // Bounded!
    if (graph.GetIncludeSize()+graph.CalcMinKeysNeeded() >= best_size) return; // Bounded!
    if (graph.GetUnknownSize() == 0) return;                          // No solution possible.

    size_t next_id = graph.GetNextID();
    emp_debug("  next_id=", next_id);
    graph_t & next_graph = graphs[depth+1];

    // if constexpr (VERBOSE) std::cout << "(Include next_id = " << next_id << ")" << std::endl;
    next_graph = graph;
    next_graph.Include(next_id);
    Solve(depth+1);

    // if constexpr (VERBOSE) std::cout << "(Exclude next_id = " << next_id << ")" << std::endl;
    next_graph = graph;
    next_graph.Exclude(next_id);
    Solve(depth+1);
  }

  void PrintStart(std::ostream & os=std::cout) const {
    os << "Keys:";
    start_graph.GetKeys().ForEach([&os](size_t id){ os << " " << id; });
    os << std::endl;
    start_graph.PrintEdges(os);
  }

  void PrintBest(std::ostream & os=std::cout) const {
    // os << "Result: " << best_graph.GetIncluded() << std::endl;
    // start_graph.PrintSubgraph(best_graph.GetIncluded(), os);
    if (solvable) {
      os << best_graph.GetIncluded().CountOnes() << std::endl;
    } else {
      os << 200 << std::endl;      
    }
  }
};