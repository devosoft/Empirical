/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  IndexMap.h
 *  @brief A simple class to weight items differently within a container and return the correct index.
 *  @note Status: BETA
 */

#ifndef EMP_INDEX_MAP_H
#define EMP_INDEX_MAP_H

#include "../base/vector.h"

namespace emp {

  /// A map of weighted indecies.  If a random index is selected, the probability of an index being
  /// returned is directly proportional to its weight.
  class IndexMap {
  private:
    emp::vector<double> item_weight;          ///< The weights associated with each ID.
    mutable emp::vector<double> tree_weight;  ///< The total weights in each sub-tree.
    mutable bool needs_refresh;               ///< Are tree weights out of date?

    /// Which ID is the parent of the ID provided?
    size_t ParentID(size_t id) const { return (id-1) / 2; }

    /// Which ID is the left child of the ID provided?
    size_t LeftID(size_t id) const { return 2*id + 1; }

    /// Which ID is the right child of the ID provided?
    size_t RightID(size_t id) const { return 2*id + 2; }

    /// Is the specified id a leaf?
    bool IsLeaf(size_t id) const { return 2*id >= item_weight.size(); }

    /// A Proxy class so that an index can be treated as an l-value.
    class Proxy {
    private:
      IndexMap & index_map;  ///< Which index map is this proxy from?
      size_t id;             ///< Which id does it represent?
    public:
      Proxy(IndexMap & _im, size_t _id) : index_map(_im), id(_id) { ; }
      operator double() const { return index_map.GetWeight(id); }
      Proxy & operator=(double new_weight) { index_map.Adjust(id, new_weight); return *this; }
    };

    /// Check if we need to do a refresh, and if so do it!
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
    /// Construct an IndexMap where num_items is the maximum number of items that can be placed
    /// into the data structure.
    IndexMap(size_t num_items=0)
      : item_weight(num_items), tree_weight(num_items), needs_refresh(false) {;}
    IndexMap(const IndexMap &) = default;
    IndexMap(IndexMap &&) = default;
    ~IndexMap() = default;
    IndexMap & operator=(const IndexMap &) = default;
    IndexMap & operator=(IndexMap &&) = default;

    /// How many indices are in this map?
    size_t GetSize() const { return item_weight.size(); }

    /// What is the total weight of all indices in this map?
    double GetWeight() const { ResolveRefresh(); return tree_weight[0]; }

    /// What is the current weight of the specified index?
    double GetWeight(size_t id) const { return item_weight[id]; }

    /// What is the probability of the specified index being selected?
    double GetProb(size_t id) const { ResolveRefresh(); return item_weight[id] / tree_weight[0]; }

    /// Change the number of indecies in the map.
    void Resize(size_t new_size) {
      const size_t old_size = item_weight.size();
      item_weight.resize(new_size, 0.0);             // Update size (new weights default to zero)
      tree_weight.resize(new_size, 0.0);             // Update size (new weights default to zero)
      if (new_size < old_size) needs_refresh = true; // Update the tree weights if some disappeared.
    }

    /// Change the number of indecies in the map, using the default weight for new indices.
    void Resize(size_t new_size, double def_value) {
      const size_t old_size = item_weight.size();
      item_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      tree_weight.resize(new_size, 0.0);   // Update the size (new weights default to zero)
      for (size_t i=old_size; i < new_size; i++) item_weight[i] = def_value;
      needs_refresh = true;                // Update the tree weights when needed.
    }

    size_t size() const { return item_weight.size(); }  ///< Standard library compatibility
    void resize(size_t new_size) { Resize(new_size); }  ///< Standard library compatibility

    /// Reset all item weights to zero.
    void Clear() {
      for (auto & x : item_weight) { x = 0.0; }
      for (auto & x : tree_weight) { x = 0.0; }
      needs_refresh = false;  // If all weights are zero, no refresh needed.
    }

    /// Change the size of this map AND change all weights to zero.
    void ResizeClear(size_t new_size) {
      item_weight.resize(new_size);
      tree_weight.resize(new_size);
      Clear();
    }

    /// Adjust the weight associated with a particular index in the map.
    /// @param id is the identification number of the item whose weight is being adjusted.
    /// @param weight is the new weight for that entry.
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

    /// Adjust all index weights to the set provided.
    void Adjust(const emp::vector<double> & new_weights) {
      item_weight = new_weights;
      needs_refresh = true;
    }

    /// Insert a new ID with the provided weight.
    size_t Insert(double in_weight) {
      size_t id = item_weight.size();
      item_weight.emplace_back(0.0);
      tree_weight.emplace_back(0.0);
      Adjust(id, in_weight);
      return id;
    }

    /// Determine the ID at the specified index position.
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

    /// Index into a specified ID.
    Proxy operator[](size_t id) { return Proxy(*this,id); }
    double operator[](size_t id) const { return item_weight[id]; }

    /// Add the weights in another index map to this one.
    IndexMap & operator+=(IndexMap & in_map) {
      emp_assert(size() == in_map.size());
      for (size_t i = 0; i < in_map.size(); i++) {
        item_weight[i] += in_map.item_weight[i];
      }
      needs_refresh = true;
      return *this;
    }

    /// Substract the weigthes from another index map from this one.
    IndexMap & operator-=(IndexMap & in_map) {
      emp_assert(size() == in_map.size());
      for (size_t i = 0; i < in_map.size(); i++) {
        item_weight[i] -= in_map.item_weight[i];
      }
      needs_refresh = true;
      return *this;
    }

    /// Indicate that we need to adjust weights before relying on them in the future; this will
    /// prevent refreshes from occuring immediately and is useful when many updates to weights are
    /// likely to be done before any are accessed again.
    void DeferRefresh() {
      needs_refresh = true;
    }

  };
}

#endif
