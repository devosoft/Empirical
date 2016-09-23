//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
// A simple class to weight items differently within a container.
//
// Constructor:
// WeightedSet(int num_items)
//     num_items is the maximum number of items that can be placed into the data structure.
//
// void Adjust(int id, double weight)
//     id is the identification number of the item whose weight is being adjusted.
//     weight is the new weight for that entry.
//
//
// Development NOTES:
//   * We are now using lazy evaluation for refreshing tree weights.  When a Refresh is run,
//     it just sets needs_refresh to true.  Any time a tree weight is requested, the refreshing
//     will actually be performed.
//
//   * We should probably change the name to something like WeightedRandom since it does not
//     have to be used just for scheduling.
//   * We could easily convert this structure to a template that acts as a glorified vector
//     giving the ability to perform a weighted random choice.
//   * We should allow the structure to be resized, either dynamically or through a Resize()
//     method.

#ifndef EMP_WEIGHTED_SET_H
#define EMP_WEIGHTED_SET_H

#include "vector.h"

namespace emp {

  class WeightedSet {
  private:
    emp::vector<double> item_weight;
    emp::vector<double> tree_weight;
    bool needs_refresh;

    int ParentID(int id) const { return (id-1) / 2; }
    int LeftID(int id) const { return 2*id + 1; }
    int RightID(int id) const { return 2*id + 2; }
    bool IsLeaf(int id) const { return 2*id >= (int) item_weight.size(); }

    class Proxy {
    private:
      WeightedSet & ws;  // Which set is this proxy from?
      int id;            // Which id does it represent?

    public:
      Proxy(WeightedSet & _ws, int _id) : ws(_ws), id(_id) { ; }

      operator double() { return ws.GetWeight(id); }
      Proxy & operator=(double new_weight) { ws.Adjust(id, new_weight); return *this; }
    };

    // Check if we need to do a refresh, and if so do it!
    void ResolveRefresh() {
      if (!needs_refresh) return;

      const int pivot = GetSize()/2 - 1; // Transition between internal and leaf nodes.
      // For a leaf, tree weight = item weight.
      for (int i = item_weight.size()-1; i > pivot; i--) tree_weight[i] = item_weight[i];
      // The pivot node may have one or two sub-trees.
      if (pivot > 0) {
        tree_weight[pivot] = item_weight[pivot] + tree_weight[LeftID(pivot)];
        if (RightID(pivot) < GetSize()) tree_weight[pivot] += tree_weight[RightID(pivot)];
      }
      // Internal nodes sum their two sub-tree and their own weight.
      for (int i = pivot-1; i >= 0; i--) {
        tree_weight[i] = item_weight[i] + tree_weight[LeftID(i)] + tree_weight[RightID(i)];
      }

      needs_refresh = false;
    }

  public:
    WeightedSet(int num_items=0)
      : item_weight(num_items), tree_weight(num_items), needs_refresh(false) {;}
    WeightedSet(const WeightedSet &) = default;
    WeightedSet(WeightedSet &&) = default;
    ~WeightedSet() = default;
    WeightedSet & operator=(const WeightedSet &) = default;
    WeightedSet & operator=(WeightedSet &&) = default;

    int GetSize() const { return (int) item_weight.size(); }
    double GetWeight() const { ResolveRefresh(); return tree_weight[0]; }
    double GetWeight(int id) const { return item_weight[id]; }
    double GetProb(int id) const { ResolveRefresh(); return item_weight[id] / tree_weight[0]; }

    void Resize(int new_size) {
      const int old_size = item_weight.size();
      item_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      tree_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      if (new_size < old_size) Refresh();  // Update the tree weights if some have disappeared.
    }

    void Resize(int new_size, double def_value) {
      const int old_size = item_weight.size();
      item_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      tree_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      for (int i=old_size; i < new_size; i++) item_weight[i] = def_value;
      Refresh();                // Update the tree weights.
    }

    // Standard library compatibility
    size_t size() const { return item_weight.size(); }
    void resize(int new_size) { Resize(new_size); }

    void Clear() {
      for (auto & x : item_weight) { x = 0.0; }
      for (auto & x : tree_weight) { x = 0.0; }
      needs_refresh = false;  // If all weights are zero, no refresh needed.
    }

    void ResizeClear(size_t new_size) {
      item_weight.resize(new_size);
      tree_weight.resize(new_size);
      Clear();
    }

    void Adjust(int id, const double new_weight) {
      // Update this node.
      const double weight_diff = new_weight - item_weight[id];
      item_weight[id] = new_weight;  // Update item weight

      if (needs_refresh) return;     // If we already need a refresh don't update tree weights!

      tree_weight[id] += weight_diff;
      // Update tree to root.
      while (id > 0) {
        id = ParentID(id);
        tree_weight[id] += weight_diff;
      }
    }

    void Adjust(const emp::vector<double> & new_weights) {
      item_weight = new_weights;
      Refresh();
    }

    size_t Insert(double in_weight) {
      size_t id = item_weight.size();
      item_weight.emplace_back(0.0);
      tree_weight.emplace_back(0.0);
      Adjust(id, in_weight);
      return id;
    }

    int Index(double index, int cur_id=0) {
      ResolveRefresh();
      emp_assert(index < tree_weight[0]);  // Cannot index beyond end of set.

      // If our target is in the current node, return it!
      const double cur_weight = item_weight[cur_id];
      if (index < cur_weight) return cur_id;

      // Otherwise determine if we need to recurse left or right.
      index -= cur_weight;
      const int left_id = LeftID(cur_id);
      const double left_weight = tree_weight[left_id];

      return (index < left_weight) ? Index(index, left_id) : Index(index-left_weight, left_id+1);
    }

    // int operator[](double index) { return Index(index,0); }
    Proxy operator[](int id) { return Proxy(*this,id); }
    double operator[](int id) const { return item_weight[id]; }

    WeightedSet & operator+=(WeightedSet & in_set) {
      emp_assert(size() == in_set.size());
      for (size_t i = 0; i < in_set.size(); i++) {
        item_weight[i] += in_set.item_weight[i];
      }
      Refresh();
      return *this;
    }
    WeightedSet & operator-=(WeightedSet & in_set) {
      emp_assert(size() == in_set.size());
      for (size_t i = 0; i < in_set.size(); i++) {
        item_weight[i] -= in_set.item_weight[i];
      }
      Refresh();
      return *this;
    }

    // Refesh used to immediately update all tree weights, but not we just indicate that we
    // need to update them before accessing them in the future.
    void Refresh() {
      needs_refresh = true;
    }

  };
};

#endif
