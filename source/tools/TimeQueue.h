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
    emp::vector<EventInfo> cell_queue;   ///< Sorted events to be triggered.
    emp::vector<EventInfo> cell_buffer;  ///< Unsorted events out of current range.

  public:
    TimeQueue(size_t _min_wait=1.0) min_wait(_min_wait) { }

    double GetTime() const { return cur_time; }
    double GetMinWait() const { return min_wait; }
    double GetSize() const { return cell_queue.size() + cell_buffer.size(); }

  };

}
