//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
// A simple class to weight items differently within a container.
//
// Constructor:
// WeightedContainer(int num_items)
//     num_items is the maximum number of items that can be placed into the data structure.
//
// void Adjust(int id, double weight)
//     id is the identification number of the item whose weight is being adjusted.
//     weight is the new weight for that entry.
//
//
// Development NOTES:
//   * We should probably change the name to something like WeightedRandom since it does not
//     have to be used just for scheduling.
//   * We could easily convert this structure to a template that acts as a glorified vector
//     giving the ability to perform a weighted random choice.
//   * We should allow the structure to be resized, either dynamically or through a Resize()
//     method.

#ifndef EMP_WEIGHTED_CONTAINER_H
#define EMP_WEIGHTED_CONTAINER_H

#include <vector>

namespace emp {

  class WeightedContainer {
  private:
    const int num_items;
    std::vector<double> weights;
    std::vector<double> tree_weights;

  public:
    WeightedContainer(int _items) : num_items(_items), weights(_items+1), tree_weights(_items+1) {
      for (int i = 0; i < (int) weights.size(); i++)  weights[i] = tree_weights[i] = 0.0;
    }
    ~WeightedContainer() = default;

    double GetWeight(int id) const { return weights[id]; }
    double GetSubtreeWeight(int id) const { return tree_weights[id]; }

    void Adjust(int id, const double _weight) {
      weights[id] = _weight;

      // Determine the child ids to adjust subtree weight.
      const int left_id = 2*id + 1;
      const int right_id = 2*id + 2;

      // Make sure the subtrees looked for haven't fallen off the end of this tree.
      const double st1_weight = (left_id < num_items) ? tree_weights[left_id] : 0.0;
      const double st2_weight = (right_id < num_items) ? tree_weights[right_id] : 0.0;
      tree_weights[id] = _weight + st1_weight + st2_weight;

      // Cascade the change up the tree to the root.
      while (id) {
        id = (id-1) / 2;
        tree_weights[id] = weights[id] + tree_weights[id*2+1] + tree_weights[id*2+2];
      }
    }

    int Index(double index, int cur_id=0) {
      // If our target is in the current node, return it!
      const double cur_weight = weights[cur_id];
      if (index < cur_weight) return cur_id;

      // Otherwise determine if we need to recurse left or right.
      index -= cur_weight;
      const int left_id = cur_id*2 + 1;
      const double left_weight = tree_weights[left_id];

      return (index < left_weight) ? Index(index, left_id) : Index(index-left_weight, left_id+1);
    }

    int operator[](double index) { return Index(index,0); }
  };
};

#endif
