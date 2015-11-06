// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_PROB_SCHEDULE_H
#define EMP_PROB_SCHEDULE_H

#include <vector>

#include "Random.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// A simple class to choose items with a probability proportional to their weight.
//
// Constructor:
// ProbSchedule(int num_items, int random_seed=-1)
//     num_items is the maximum number of items that can be placed into the data structure.
//     random_seed is the seed for the random number generator used; values < 0 base seed on time.
//
// void Adjust(int id, double weight)
//     id is the identification number of the item whose weight is being adjusted.
//     weight is the new weight for that entry.
//
// int NextID() returns a random id based on the weights provided.
//
//
// Development NOTES:
//   * We should probably change the name to something like WeightedRandom since it does not
//     have to be used just for scheduling.
//   * We could easily convert this structure to a template that acts as a glorified vector
//     giving the ability to perform a weighted random choice.
//   * We should allow the structure to be resized, either dynamically or through a Resize()
//     method.
//


namespace emp {

  class ProbSchedule {
  private:
    const int num_items;
    std::vector<double> weights;
    std::vector<double> tree_weights;
    Random m_rng;
    
    ProbSchedule(const ProbSchedule&); // @not_implemented
    ProbSchedule& operator=(const ProbSchedule&); // @not_implemented
    
    int CalcID(double rand_pos, int cur_id) {
      // If our target is in the current node, return it!
      const double cur_weight = weights[cur_id];
      if (rand_pos < cur_weight) return cur_id;

      // Otherwise determine if we need to recurse left or right.
      rand_pos -= cur_weight;
      const int left_id = cur_id*2 + 1;
      const double left_weight = tree_weights[left_id];

      return (rand_pos < left_weight) ? CalcID(rand_pos, left_id) : CalcID(rand_pos-left_weight, left_id+1);
    }
    
  public:
    ProbSchedule(int _items, int seed=-1) : num_items(_items), weights(_items+1), tree_weights(_items+1), m_rng(seed) {
      for (int i = 0; i < (int) weights.size(); i++)  weights[i] = tree_weights[i] = 0.0;
    }
    ~ProbSchedule() { ; }
 
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
    
    int NextID() {
      const double total_weight = tree_weights[0];

      // Make sure it's possible to schedule...
      if (total_weight == 0.0) return -1;
      
      // If so, choose a random number to use for the scheduling.
      double rand_pos = m_rng.GetDouble(total_weight);
      return CalcID(rand_pos, 0);
    }
  };
};

#endif
