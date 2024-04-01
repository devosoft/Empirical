#include <fstream>
#include <istream>
#include <ostream>

#include "../../../include/emp/base/vector.hpp"
#include "Graph.hpp"
#include "Solver.hpp"

void Load(Graph<256> & graph, std::istream & is) {
  // First line is n, m, and k.
  size_t n, m, k;
  is >> n >> m >> k;
  graph.Resize(n);

  // Next m lines are edges.
  size_t from, to;
  for (size_t i = 0; i < m; ++i) {
    is >> from >> to;
    graph.AddEdge(from, to);
  }

  // Final k lines are keys.
  size_t key_id;
  for (size_t i = 0; i < k; ++i) {
    is >> key_id;
    graph.SetKey(key_id);
  }
}

int main(int argc, char * argv[])
{
  Graph<256> graph;
  if (argc > 1) {
    std::ifstream file(argv[1]);
    Load(graph, file);
  } else {
    Load (graph, std::cin);
  }

  graph.Setup();
  // graph.PrintMatrix();
  // graph.PrintEdges();
  // graph.Optimize();
  // std::cout << "After opts:\n";
  // graph.PrintEdges();

  Solver solver(graph);
  // solver.PrintStart();
  solver.Solve();
  solver.PrintBest();
  // solver.Debug();
}