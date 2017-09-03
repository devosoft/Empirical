//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple class to choose items with a probability proportional to their weight.
//  Status: BETA
//
//  Constructor:
//  ProbSchedule(size_t num_items, int random_seed=-1)
//      num_items is the maximum number of items that can be placed into the data structure.
//      random_seed is the seed for the random number generator used; values < 0 base seed on time.
//
//  void Adjust(size_t id, double weight)
//      id is the identification number of the item whose weight is being adjusted.
//      weight is the new weight for that entry.
//
//  size_t NextID() returns a random id based on the weights provided.
//
//
//  Development NOTES:
//   * We should probably change the name to something like WeightedRandom since it does not
//     have to be used just for scheduling.
//   * We could easily convert this structure to a template that acts as a glorified vector
//     giving the ability to perform a weighted random choice.
//   * We should allow the structure to be resized, either dynamically or through a Resize()
//     method.

#ifndef EMP_PROB_SCHEDULE_H
#define EMP_PROB_SCHEDULE_H

#include "Random.h"
#include "vector"

namespace emp {

  /// A simple class to choose items with a probability proportional to their weight.
  class ProbSchedule {
  private:
    const size_t num_items;
    emp::vector<double> weights;
    emp::vector<double> tree_weights;
    Random m_rng;

    ProbSchedule(const ProbSchedule&); // @not_implemented
    ProbSchedule& operator=(const ProbSchedule&); // @not_implemented

    size_t CalcID(double rand_pos, size_t cur_id) {
      // If our target is in the current node, return it!
      const double cur_weight = weights[cur_id];
      if (rand_pos < cur_weight) return cur_id;

      // Otherwise determine if we need to recurse left or right.
      rand_pos -= cur_weight;
      const size_t left_id = cur_id*2 + 1;
      const double left_weight = tree_weights[left_id];

      return (rand_pos < left_weight) ? CalcID(rand_pos, left_id) : CalcID(rand_pos-left_weight, left_id+1);
    }

  public:

    /// num_items is the maximum number of items that can be placed into the data structure.
    /// random_seed is the seed for the random number generator used; values < 0 base seed on time.
    ProbSchedule(size_t _items, int seed=-1) : num_items(_items), weights(_items+1), tree_weights(_items+1), m_rng(seed) {
      for (size_t i = 0; i < weights.size(); i++)  weights[i] = tree_weights[i] = 0.0;
    }
    ~ProbSchedule() { ; }

    size_t GetSize() const { return num_items; }

    // Standard library compatibility
    size_t size() const { return num_items; }

    double GetWeight(size_t id) const { return weights[id]; }
    double GetSubtreeWeight(size_t id) const { return tree_weights[id]; }


    void Adjust(size_t id, const double _weight) {
      weights[id] = _weight;

      // Determine the child ids to adjust subtree weight.
      const size_t left_id = 2*id + 1;
      const size_t right_id = 2*id + 2;

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

    /// Returns a random id based on the weights provided.
    int NextID() {
      const double total_weight = tree_weights[0];

      // Make sure it's possible to schedule...
      if (total_weight == 0.0) return -1;

      // If so, choose a random number to use for the scheduling.
      double rand_pos = m_rng.GetDouble(total_weight);
      return (int) CalcID(rand_pos, 0);
    }
  };
}

#endif
