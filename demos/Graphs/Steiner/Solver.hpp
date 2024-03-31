#pragma once

#include "../../../include/emp/base/vector.hpp"

#include "Graph.hpp"

class Solver {
private:
  static constexpr bool VERBOSE = false;

  Graph start_graph;
  Graph best_graph;
  size_t best_size = 10000000;

  void MarkSolution(const Graph & graph) {
    if (graph.GetIncludeSize() < best_size) {
      best_graph = graph;
      best_size = graph.GetIncludeSize();
      std::cout << "New Best: " << best_size << std::endl;
    }
  }

  void PrintSolveState(const Graph & graph, size_t depth) const {
    for (size_t i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << "Solve(" << depth << "):"
              << " keys:" << graph.GetKeys()
              << " included:" << graph.GetIncluded()
              << " unknown:" << graph.GetUnknown()
              << " solved:" << graph.IsSolved()
              << std::endl;    
  }

public:
  Solver(const Graph & graph) : start_graph(graph) { }

  void Debug() {
    PrintSolveState(start_graph, 0);
    start_graph.Include(0);
    PrintSolveState(start_graph, 0);
  }

  void Solve() {
    Graph graph(start_graph);
    graph.Setup();
    Solve(graph, 0);
  }

  void Solve(Graph graph, size_t depth) {
    emp_debug("SOLVE: ", depth, " key=", graph.GetKeys(), " included=", graph.GetIncluded(), " unknown=", graph.GetUnknown());

    if constexpr (VERBOSE) PrintSolveState(graph, depth);

    graph.Optimize();

    if (graph.IsSolved()) { MarkSolution(graph); return; }  // Solved!
    if (graph.GetIncludeSize()+1 >= best_size) return;      // Bounded!
    if (graph.GetUnknownSize() == 0) return;                // No solution possible.

    size_t next_id = graph.GetNextID();
    emp_debug("  next_id=", next_id);
    // if constexpr (VERBOSE) std::cout << "(Exclude next_id = " << next_id << ")" << std::endl;
    graph.Exclude(next_id);
    Solve(graph, depth+1);
    // if constexpr (VERBOSE) std::cout << "(Include next_id = " << next_id << ")" << std::endl;
    graph.Include(next_id);
    Solve(graph, depth+1);
  }

  void PrintStart(std::ostream & os=std::cout) const {
    os << "Keys:";
    start_graph.GetKeys().ForEach([&os](size_t id){ os << " " << id; });
    os << std::endl;
    start_graph.PrintEdges(os);
  }

  void PrintBest(std::ostream & os=std::cout) const {
    os << "Result: " << best_graph.GetIncluded() << std::endl;
    start_graph.PrintSubgraph(best_graph.GetIncluded(), os);
    os << best_graph.GetIncluded().CountOnes() << std::endl;
  }
};