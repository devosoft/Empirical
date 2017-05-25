//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple class to weight items differently within a container and return the correct index.
//  Status: BETA
//
//
//  Constructor:
//  IndexMap(size_t num_items)
//     num_items is the maximum number of items that can be placed into the data structure.
//
//  void Adjust(size_t id, double weight)
//     id is the identification number of the item whose weight is being adjusted.
//     weight is the new weight for that entry.

#ifndef EMP_INDEX_MAP_H
#define EMP_INDEX_MAP_H

#include "../base/vector.h"

namespace emp {

  class IndexMap {
  private:
    emp::vector<double> item_weight;
    mutable emp::vector<double> tree_weight;
    mutable bool needs_refresh;

    size_t ParentID(size_t id) const { return (id-1) / 2; }
    size_t LeftID(size_t id) const { return 2*id + 1; }
    size_t RightID(size_t id) const { return 2*id + 2; }
    bool IsLeaf(size_t id) const { return 2*id >= item_weight.size(); }

    class Proxy {
    private:
      IndexMap & index_map;  // Which index map is this proxy from?
      size_t id;             // Which id does it represent?
    public:
      Proxy(IndexMap & _im, size_t _id) : index_map(_im), id(_id) { ; }
      operator double() const { return index_map.GetWeight(id); }
      Proxy & operator=(double new_weight) { index_map.Adjust(id, new_weight); return *this; }
    };

    // Check if we need to do a refresh, and if so do it!
    void ResolveRefresh() const {
      if (!needs_refresh) return;

      const size_t pivot = GetSize()/2 - 1; // Transition between internal and leaf nodes.
      // For a leaf, tree weight = item weight.
      for (size_t i = item_weight.size()-1; i > pivot; i--) tree_weight[i] = item_weight[i];
      // The pivot node may have one or two sub-trees.
      if (pivot > 0) {
        tree_weight[pivot] = item_weight[pivot] + tree_weight[LeftID(pivot)];
        if (RightID(pivot) < GetSize()) tree_weight[pivot] += tree_weight[RightID(pivot)];
      }
      // Internal nodes sum their two sub-tree and their own weight.
      for (size_t i = pivot-1; i < pivot; i--) {
        tree_weight[i] = item_weight[i] + tree_weight[LeftID(i)] + tree_weight[RightID(i)];
      }

      needs_refresh = false;
    }

  public:
    IndexMap(size_t num_items=0)
      : item_weight(num_items), tree_weight(num_items), needs_refresh(false) {;}
    IndexMap(const IndexMap &) = default;
    IndexMap(IndexMap &&) = default;
    ~IndexMap() = default;
    IndexMap & operator=(const IndexMap &) = default;
    IndexMap & operator=(IndexMap &&) = default;

    size_t GetSize() const { return item_weight.size(); }
    double GetWeight() const { ResolveRefresh(); return tree_weight[0]; }
    double GetWeight(size_t id) const { return item_weight[id]; }
    double GetProb(size_t id) const { ResolveRefresh(); return item_weight[id] / tree_weight[0]; }

    void Resize(size_t new_size) {
      const size_t old_size = item_weight.size();
      item_weight.resize(new_size, 0.0);             // Update size (new weights default to zero)
      tree_weight.resize(new_size, 0.0);             // Update size (new weights default to zero)
      if (new_size < old_size) needs_refresh = true; // Update the tree weights if some disappeared.
    }

    void Resize(size_t new_size, double def_value) {
      const size_t old_size = item_weight.size();
      item_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      tree_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      for (size_t i=old_size; i < new_size; i++) item_weight[i] = def_value;
      needs_refresh = true;                // Update the tree weights when needed.
    }

    // Standard library compatibility
    size_t size() const { return item_weight.size(); }
    void resize(size_t new_size) { Resize(new_size); }

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

    void Adjust(size_t id, const double new_weight) {
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
      needs_refresh = true;
    }

    size_t Insert(double in_weight) {
      size_t id = item_weight.size();
      item_weight.emplace_back(0.0);
      tree_weight.emplace_back(0.0);
      Adjust(id, in_weight);
      return id;
    }

    size_t Index(double index, size_t cur_id=0) const {
      ResolveRefresh();

      // Make sure we don't try to index beyond end of map.
      emp_assert(index < tree_weight[0], index, tree_weight.size(), tree_weight[0]);

      // If our target is in the current node, return it!
      const double cur_weight = item_weight[cur_id];
      if (index < cur_weight) return cur_id;

      // Otherwise determine if we need to recurse left or right.
      index -= cur_weight;
      const size_t left_id = LeftID(cur_id);
      const double left_weight = tree_weight[left_id];

      return (index < left_weight) ? Index(index, left_id) : Index(index-left_weight, left_id+1);
    }

    // size_t operator[](double index) { return Index(index,0); }
    Proxy operator[](size_t id) { return Proxy(*this,id); }
    double operator[](size_t id) const { return item_weight[id]; }

    IndexMap & operator+=(IndexMap & in_map) {
      emp_assert(size() == in_map.size());
      for (size_t i = 0; i < in_map.size(); i++) {
        item_weight[i] += in_map.item_weight[i];
      }
      needs_refresh = true;
      return *this;
    }
    IndexMap & operator-=(IndexMap & in_map) {
      emp_assert(size() == in_map.size());
      for (size_t i = 0; i < in_map.size(); i++) {
        item_weight[i] -= in_map.item_weight[i];
      }
      needs_refresh = true;
      return *this;
    }

    // Indicate that we need to adjust weights before relying on them in the future; this will
    // prevent refreshes from occuring immediately and is useful when many updates to weights are
    // likely to be done before any are accessed again.
    void DeferRefresh() {
      needs_refresh = true;
    }

  };
};

#endif
