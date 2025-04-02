#pragma once

#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/bits/BitVector.hpp"
#include "../../../include/emp/debug/debug.hpp"

template <size_t MAX_BITS>
class Graph {
private:
  using bits_t = emp::StaticBitVector<MAX_BITS>;

  emp::vector<bits_t> edges;
  bits_t keys;       // Key vertices that we still need to merge.
  bits_t included;   // Current and former key vertices that are included in the solution.
  bits_t unknown;    // Vertices that we still need to consider for the solution.

public:
  Graph() { }
  Graph(size_t node_count) { Resize(node_count); }
  Graph(const Graph &) = default;
  Graph(Graph &&) = default;

  Graph & operator=(const Graph &) = default;
  Graph & operator=(Graph &&) = default;

  bool HasKey(size_t id) const { return keys.Has(id); }
  bool HasEdge(size_t id1, size_t id2) const { return edges[id1].Has(id2); }
  size_t GetDegree(size_t id) const { return edges[id].CountOnes(); }
  size_t GetKeyDegree(size_t id) const { return (edges[id] & keys).CountOnes(); }

  size_t GetVertexCount() const { return included.size(); }
  size_t GetKeyCount() const { return keys.CountOnes(); }
  size_t GetIncludeSize() const { return included.CountOnes(); }
  size_t GetUnknownSize() const { return unknown.CountOnes(); }
  bool IsSolved() const { return GetKeyCount() == 1; }

  bool IsUnknown(size_t id) const { return unknown[id]; }
  bool IsIncluded(size_t id) const { return included[id]; }
  bool IsKey(size_t id) const { return keys[id]; }
  bool IsActive(size_t id) const { return IsUnknown(id) || IsKey(id); }

  const bits_t & GetIncluded() const { return included; }
  const bits_t & GetKeys() const { return keys; }
  const bits_t & GetUnknown() const { return unknown; }

  // size_t GetNextID() const { return static_cast<size_t>(unknown.FindOne()); }
  size_t GetNextID() const {
    // return unknown.FindOne();
    // return unknown.MaxIndex([this](size_t id){ return GetDegree(id); });
    return unknown.MaxIndex([this](size_t id){ return GetKeyDegree(id)*5 +  GetDegree(id); });
  }

  // Get a connection to a neighbor OTHER than the one specified.
  size_t GetOtherNeighbor(size_t target_id, size_t known_id) const {
    size_t out = edges[target_id].FindOne();
    if (out == known_id) out = edges[target_id].FindOne(known_id+1);
    return out;
  }

  bool IsUnsolvable() const {
    if (IsSolved()) return false;
    for (int i = keys.FindOne(); i >= 0; i = keys.FindOne(i+1)) {
      if (GetDegree(i) == 0) return true;
    }
    return false;
  }

  bits_t CalcReachable(size_t start_id) const {
    bits_t found(keys.size()), explored(keys.size());
    found.Set(start_id);

    while (found) {
      size_t next_id = found.FindOne();  // Grab the next found vertex
      found |= edges[next_id];           // Explore all of its edges
      explored.set(next_id);             // Make it as explored
      found &= ~explored;                // Remove all explored nodes from found
    }

    return explored;
  }

  bool TestSolvable() const {
    if (keys.None()) return true; // Trivially solvable.
    bits_t reachable = CalcReachable(keys.FindOne());
    // std::cout << "Keys:             " << keys << std::endl;
    // std::cout << "Reachable:        " << reachable << std::endl;
    // std::cout << "Unreachable keys: " << (keys & ~reachable) << std::endl;
    return !(keys & ~reachable);
  }

  void Resize(size_t new_size) {
    edges.resize(new_size);
    for (auto & v : edges) v.resize(new_size);
    keys.resize(new_size);
    included.resize(new_size);
    unknown.resize(new_size);
    unknown.SetAll();
  }

  void AddEdge(size_t id1, size_t id2) {
    if (id1 != id2) {
      edges[id1].Set(id2);
      edges[id2].Set(id1);
    }
  }

  void RemoveEdge(size_t id1, size_t id2) {
    edges[id1].Clear(id2);
    edges[id2].Clear(id1);
  }

  void SetKey(size_t id) {
    keys.Set(id);
    included.Set(id);
    unknown.Clear(id);
  }

  void EraseNode(size_t id) {
    // Remove all of the edges associated with this node.
    while (edges[id].Any()) {
      RemoveEdge(id, edges[id].FindOne());
    }

    // Remove other records of this node.
    keys.Clear(id); // If id was a key, it shouldn't be anymore.
    // Leave as included.
    unknown.Clear(id);
  }

  // Collapse the second node into the first.
  void MergeNodes(size_t id1, size_t id2) {
    if (id1 == id2) return; // Cannot merge a node with itself.
    auto new_nodes = edges[id2] & ~edges[id1];  // New nodes that id1 should be connected to.
    while (new_nodes) {
      AddEdge(id1, static_cast<size_t>(new_nodes.PopOne()));
    }
    EraseNode(id2);
  }

  bool MergeKeysTo(size_t id) {
    bool modified = false;
    while (edges[id] & keys) {
      MergeNodes(id, (edges[id] & keys).FindOne());
      modified = true;
    }
    return modified;
  }

  bool DoMerges() {
    bool modified = false;

    // If any neighboring vertices are both key, merge them together.
    for (size_t i = 0; i < keys.size(); ++i) {
      if (!keys.Has(i)) continue;
      modified |= MergeKeysTo(i);
    }

    return modified;
  }

  // Trim a degree 1 vertex.
  void TrimDegree1(size_t id) {
    emp_assert(GetDegree(id) == 1, id);

    // If it is a key vertex, it must move through its neighbor.
    if (keys.Has(id)) MergeNodes(id, edges[id].FindOne());

    // Otherwise it's unneeded.
    else EraseNode(id);
  }

  // Trim a degree 2 vertex that has it's two neighbors connecting
  bool ReduceDegree2(size_t id) {
    emp_assert(GetDegree(id) == 2, id);
    int id1 = edges[id].FindOne();
    int id2 = edges[id].FindOne(id1+1);

    if (HasEdge(id1, id2)) {
      // If it is a key, disconnect neighbors (can just go through it)
      if (keys.Has(id)) RemoveEdge(id1, id2);
      else EraseNode(id);
      return true;
    }

    return false;
  }


  // Run at the beginning of solving to clean up graph.
  void Setup() {
    // Include all of the keys.
    included = keys;
    unknown = ~included;
  }

  void Include(size_t id) {
    SetKey(id);
    MergeKeysTo(id); // If this new vertex connects to other keys, merge them in.
  }

  void Exclude(size_t id) {
    emp_assert(id <= unknown.size(), id, unknown.size());
    unknown.Clear(id);    
  }

  bool OptimizeDegrees() {
    bool progress = false;

    // First scan through active keys.
    for (size_t i = keys.FindOne(); i < keys.size() && GetKeyCount() > 1; i = keys.FindOne(i+1)) {
      switch (GetDegree(i)) {
      case 0:
        return false; // No need for more optimizations; we are already done.
      case 1:
        emp_debug("OPT: ", i, " is KEY degree one; removing and setting ", edges[i].FindOne(), " as key.");
        SetKey( edges[i].FindOne() );  // Set the neighbor as key before removing the current vertex.
        EraseNode(i);
        progress = true;
        break;
      case 2:
        // Degree 2 can remove an opposite edge.
        const size_t n1 = edges[i].FindOne();
        const size_t n2 = edges[i].FindOne(n1+1);
        if (HasEdge(n1, n2)) {
          RemoveEdge(n1, n2);
          progress = true;
          break;
        }

        // Can also follow a series of degree 2 vertices to key and connect in closer.
        size_t dist1 = 1;
        size_t prev_id = i;
        size_t next_id = n1;
        while (IsKey(next_id) == false && GetDegree(next_id) == 2) {
          prev_id = GetOtherNeighbor(next_id, prev_id);
          std::swap(prev_id, next_id);
          ++dist1;
        }
        if (!IsKey(next_id)) break; // If we didn't make it to a key, no optimization.

        size_t dist2 = 1;
        prev_id = i;
        next_id = n2;
        while (IsKey(next_id) == false && GetDegree(next_id) == 2) {
          prev_id = GetOtherNeighbor(next_id, prev_id);
          std::swap(prev_id, next_id);
          ++dist2;
        }
        if (!IsKey(next_id)) break; // If we didn't make it to a key, no optimization.

        // We now have the two distances; connect to the closest!
        if (dist1 <= dist2) SetKey(n1);
        else SetKey(n2);
        progress = true;
        break;
      }
    }

    // Then scan through active non-keys.
    for (size_t i = unknown.FindOne(); i < unknown.size(); i = unknown.FindOne(i+1)) {
      switch (GetDegree(i)) {
      case 0:
      case 1:
        emp_debug("OPT: ", i, " is NON-key degree zero or one; removing.");
        EraseNode(i);
        progress = true;
        break;
      case 2:  // See if the neighbors have another connection; if so delete this one.
        size_t n1 = edges[i].FindOne();
        size_t n2 = edges[i].FindOne(n1+1);
        if ( (edges[n1] & edges[n2]).CountOnes() > 1 ) {
          emp_debug("OPT: ", i, " is degree two (n1=", n1, " n2=", n2, "; shared=", (edges[n1] & edges[n2]).CountOnes(), ") - REMOVING ");
          EraseNode(i);
          progress = true;
        }
      }
    }

    return progress;
  }

  // Run through all optimizations.
  bool Optimize() {
    bool progress = false;
    bool check_opts = true;
    while (check_opts) {
      check_opts = false;
      check_opts |= OptimizeDegrees();
      check_opts |= DoMerges();
      progress |= check_opts;
    }
    return progress;
  }

  // Place a lower bound on the number of additional keys that will be needed.
  size_t CalcMinKeysNeeded() const {
    // Find the maximum number of keys a single node can link in.
    size_t max_id = unknown.MaxIndex([this](size_t id){ return GetKeyDegree(id); });
    return (GetKeyCount()-1) / GetKeyDegree(max_id) + 1;
  }

  bool TestOneNodeSolution() {
    size_t id = unknown.FindIndex([this](size_t id){ return (edges[id] & keys) == keys; });
    if (id < unknown.size()) { SetKey(id); return true; }
    return false;
  }

  bool TestTwoNodeSolution() {
    return unknown.HasIndexPair([this](size_t id1, size_t id2) {
      if (((edges[id1] | edges[id2]) & keys) == keys) { // Do these two nodes cover everything?
        if (HasEdge(id1, id2) || (edges[id1] & edges[id2] & keys)) { // Are the groups connected?
          SetKey(id1);
          SetKey(id2);
          return true;
        }
      }
      return false;
    });
  }

  void PrintMatrix(std::ostream & os=std::cout) const {
    for (size_t i = 0; i < edges.size(); ++i) {
      os << i << " : " << edges[i] << std::endl;
    }
  }

  void PrintEdges(std::ostream & os=std::cout) const {
    auto active = unknown | keys;
    for (size_t id1 = active.FindOne(); id1 < active.size(); id1 = active.FindOne(id1+1)) {
      if (GetDegree(id1) == 0) {
        os << "[";
        if (IsKey(id1)) os << "*";
        os << id1 << "] ";
        continue;
      }
      for (size_t id2 = active.FindOne(id1+1); id2 < active.size(); id2 = active.FindOne(id2+1)) {
        if (HasEdge(id1, id2)) {
          if (IsKey(id1)) os << "*";
          os << id1 << "->";
          if (IsKey(id2)) os << "*";
          os << id2 << " ";
        }
      }
    }

    // Print included, but inactive nodes
    auto node_set = included & ~active;
    if (node_set.Any()) {
      os << "Plus " << node_set.CountOnes() << " included: ";
      for (size_t id = node_set.FindOne(); id < node_set.size(); id = node_set.FindOne(id+1)) {
        os << id << " ";
      }      
    }
    os << std::endl;
  }

  /// Print only the portion of the graph with the provided nodes.
  void PrintSubgraph(bits_t nodes, std::ostream & os=std::cout) const {
    nodes.ForEachPair([this, &os](size_t id1, size_t id2){
      if (HasEdge(id1,id2)) {
        os << " " << id1 << "->" << id2;
      }
    });
    os << std::endl;
  }
};