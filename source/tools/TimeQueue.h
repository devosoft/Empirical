/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  TimeQueue.h
 *  @brief A priority queue for timings, always marching forward.
 *  @note Status: ALPHA
 *
 */


#ifndef EMP_TIME_QUEUE_H
#define EMP_TIME_QUEUE_H

#include "../base/assert.h"
#include "../base/vector.h"

namespace emp {

  template <typename T>
  class TimeQueue {
  private:
    struct ItemInfo {
      T item;
      double timing;
  
      bool operator<(const ItemInfo & _in) const {
        if (timing == _in.timing) return (item < _in.item);
        return (timing < _in.timing);
      }
    } 

    double cur_time = 0.0;               ///< What time are we up to?
    double min_wait = 1.0;               ///< Minimum amount of time for the next event.
    emp::vector<EventInfo> item_queue;   ///< Sorted events to be triggered.
    emp::vector<EventInfo> item_buffer;  ///< Unsorted events out of current range.

    // Helper function to move more items into the queue.
    bool RefillQueue() {
      emp_assert(item_queue.size() == 0);
      if (item_buffer.size() == 0) return false;  // No cells left!

      // Scan the buffer to determine the earliest time.
      const double first_time = FindMin(item_buffer).timing;

      // Move all cells that will replicate in the next "min_wait" timesteps to item_queue.
      double last_time = first_time + min_wait;
      size_t keep_count = 0;
      for (size_t i = 0; i < item_buffer.size(); i++) {
        if (item_buffer[i].timing <= last_time) item_queue.push_back(item_buffer[i]);
        else item_buffer[keep_count++] = item_buffer[i];
      }
      item_buffer.resize(keep_count);

      // Sort the cell queue so that it's ready to go.
      emp::Sort(item_queue);

      return true;
    }
  public:
    TimeQueue(size_t _min_wait=1.0) min_wait(_min_wait) {
      emp_assert(min_wait > 0.0);
    }

    double GetTime() const { return cur_time; }
    double GetMinWait() const { return min_wait; }
    double GetSize() const { return cell_queue.size() + cell_buffer.size(); }

  };

}
