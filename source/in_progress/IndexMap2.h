/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  IndexMap.h
 *  @brief A simple class to weight items differently within a container and return the correct index.
 *  @note Status: BETA
 *
 *  @todo Convert to a template that acts as a glorified vector, simplifying random selection?
 *  @todo Should operator[] index by element count or by weight?
 */

#ifndef EMP_INDEX_MAP_H
#define EMP_INDEX_MAP_H

#include "../base/vector.h"

namespace emp {

  /// A map of weighted indecies.  If a random index is selected, the probability of an index being
  /// returned is directly proportional to its weight.
  class IndexMap {
  private:
    // Internally, item_weight should be thought of as following tree_weight as a vector, both of
    // which are the length of the number of values stored.  Note that portions are mutable so
    // that we can do a lazy updating of tree_weight when needs_refresh is set.

    size_t num_items;                      ///< How many items are being stores in this IndexMap?
    mutable bool needs_refresh;            ///< Are tree weights out of date?
    mutable emp::vector<double> weights;   ///< The total weights in each sub-tree.

    /// Which ID is the parent of the ID provided?
    size_t ParentID(size_t id) const { return (id-1) / 2; }

    /// Which ID is the left child of the ID provided?
    size_t LeftID(size_t id) const { return 2*id + 1; }

    /// Which ID is the right child of the ID provided?
    size_t RightID(size_t id) const { return 2*id + 2; }

    /// A Proxy class so that an index can be treated as an l-value.
    class Proxy {
    private:
      IndexMap & index_map;  ///< Which index map is this proxy from?
      size_t id;             ///< Which id does it represent?
    public:
      Proxy(IndexMap & _im, size_t _id) : index_map(_im), id(_id) { ; }
      operator double() const { return index_map.RawWeight(id); }
      Proxy & operator=(double new_weight) { index_map.RawAdjust(id, new_weight); return *this; }
    };

    /// Check if we need to do a refresh, and if so do it!
    void ResolveRefresh() const {
      if (!needs_refresh) return;

      // Internal nodes sum their two sub-trees.
      const size_t pivot = num_items - 2; // Transition between internal and leaf nodes.
      for (size_t i = pivot; i < pivot; i--) {
        weights[i] = weights[LeftID(i)] + weights[RightID(i)];
      }

      needs_refresh = false;
    }

  public:
    /// Construct an IndexMap where num_items is the maximum number of items that can be placed
    /// into the data structure.  All item weigths default to zero.
    IndexMap(size_t _items=0)
      : num_items(_items), weights(_items*2-1,0), needs_refresh(false) {;}
    IndexMap(size_t _items, double init_weight)
      : num_items(_items), weights(num_items, init_weight), needs_refresh(true) { ; }
    IndexMap(const IndexMap &) = default;
    IndexMap(IndexMap &&) = default;
    ~IndexMap() = default;
    IndexMap & operator=(const IndexMap &) = default;
    IndexMap & operator=(IndexMap &&) = default;

    /// How many indices are in this map?
    size_t GetSize() const { return num_items; }

    /// What is the total weight of all indices in this map?
    double GetWeight() const { ResolveRefresh(); return weights[0]; }

    /// What is the current weight of the specified index?
    double RawWeight(size_t id) const { return weights[id]; }
    double GetWeight(size_t id) const { return RawWeight(id+num_items-1); }

    /// What is the probability of the specified index being selected?
    double RawProb(size_t id) const { ResolveRefresh(); return weights[id] / weights[0]; }
    double GetProb(size_t id) const { return RawProb(id+num_items-1); }

    /// Change the number of indecies in the map.
    void Resize(size_t new_size, double def_value=0.0) {
      const size_t min_size = std::min(num_items, new_size);

      emp::vector<double> new_weights(2*new_size - 1);
      for (size_t i = 0; i < min_size; i++) {
        new_weights[new_size - 1 + i] = weights[num_items - 1 + i];
      }
      for (size_t i = min_size; i < new_size; i++) {
        new_weights[new_size - 1 + i] = def_value;
      }
      needs_refresh = true;                      // Update the tree weights when needed.
      num_items = new_size;
    }

    size_t size() const { return num_items; }  ///< Standard library compatibility
    void resize(size_t new_size) { Resize(new_size); }  ///< Standard library compatibility

    /// Reset all item weights to zero.
    void Clear() {
      for (auto & x : weights) { x = 0.0; }
      needs_refresh = false;  // If all weights are zero, no refresh needed.
    }

    /// Change the size of this map AND change all weights to zero.
    void ResizeClear(size_t new_size) {
      weights.resize(2*new_size - 1);
      num_items = new_size;
      Clear();
    }

    /// Adjust the weight associated with a particular index in the map.
    /// @param id is the identification number of the item whose weight is being adjusted.
    /// @param weight is the new weight for that entry.
    void RawAdjust(size_t id, const double new_weight) {
      // Update this node.
      const double weight_diff = new_weight - weights[id];
      weights[id] = new_weight;    // Update item weight

      if (needs_refresh) return;     // If we already need a refresh don't update tree weights!

      // Update tree to root.
      while (id > 0) {
        id = ParentID(id);
        weights[id] += weight_diff;
      }
    }

    void Adjust(size_t id, const double new_weight) { RawAdjust(id + num_items - 1, new_weight); }

    /// Adjust all index weights to the set provided.
    void Adjust(const emp::vector<double> & new_weights) {
      num_items = new_weights.size();
      weights.resize(num_items*2 - 1);
      for (size_t i = 0; i < num_items; i++) weights[num_items - 1 + i]  = new_weights[i];
      needs_refresh = true;
    }

    /// Adjust all index weights to the set provided.
    void AdjustAll(double new_weight) {
      for (auto & x : weights) x = new_weight;
      needs_refresh = true;
    }

    /// Determine the ID at the specified index position.
    size_t Index(double index, size_t cur_id=0) const {
      ResolveRefresh();

      // Make sure we don't try to index beyond end of map.
      emp_assert(index < weights[cur_id], index, cur_id, weights.size(), weights[cur_id]);

      // If we are on a leaf, we have our answer!
      if (cur_id >= num_items - 1) return cur_id - (num_items - 1);

      const size_t left_id = LeftID(cur_id);
      const double left_weight = weights[left_id];

      if (index < left_weight) return Index(index, left_id);
      return Index(index-left_weight, left_id+1);
    }

    // size_t operator[](double index) { return Index(index,0); }

    /// Index into a specified ID.
    Proxy operator[](size_t id) { return Proxy(*this,id); }
    double operator[](size_t id) const { return weights[id+num_items-1]; }

    /// Add the weights in another index map to this one.
    IndexMap & operator+=(IndexMap & in_map) {
      emp_assert(size() == in_map.size());
      for (size_t i = 0; i < in_map.size(); i++) {
        weights[i+num_items-1] += in_map.weights[i+num_items-1];
      }
      needs_refresh = true;
      return *this;
    }

    /// Substract the weigthes from another index map from this one.
    IndexMap & operator-=(IndexMap & in_map) {
      emp_assert(size() == in_map.size());
      for (size_t i = 0; i < in_map.size(); i++) {
        weights[i+num_items-1] -= in_map.weights[i+num_items-1];
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
