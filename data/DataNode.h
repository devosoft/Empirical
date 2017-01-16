//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  DataNode objects track a specific type of data over the course of a run.
//
//  @CAO: Pehaps each of the functions below can be part of an add-on module?
//
//  Collection: New data can be pushed or pulled.
//   Add(VAL... v) pushes data to a node
//   AddDatum(VAL v) pushes just one datum, but can be used as an action for a signal.
//   @CAO: Should Add trigger a signal to inform others that a datum has been collected?

//   @CAO: PullData() triggers a signal that can be monitored to collect data.
//
//  Storage: Three types of storage. By default, the stored type may be anything, but stats
//           require a type that math can be performed on.
//   Current  : Keep the most recent input value and a count of inputs since the last Reset()
//   Stats    : Maintain basic stats (min, max, input count, mean, standard deviation,
//               skew, kertosis), as values are supplied, but not individual values.
//   Full     : Stores ALL values that are provided since last Reset()
//
//  Process: What should happen on Reset() ?
//   * Trigger an actions to process the prior update's data stored.
//   * Clear all data.
//   * Send data to a stream
//     (or stats automatically have a stream that, if non-null data is sent to?)

#ifndef EMP_DATA_NODE_H
#define EMP_DATA_NODE_H

#include "../tools/assert.h"

namespace emp {

  // A set of modifiers are available do describe DataNode
  namespace data {
    class Current;   // Track most recent value

    class Log;       // Track all values since last Reset()
    // class Prev;      // Track Log + values from previous Reset().
    // class Archive;   // Track Log + ALL values over time (with purge options)

    class Range;     // Track min, max, mean, total
    // class Stats;     // Track Range + variance, standard deviation, skew, kertosis
    // class FullStats; // Track States + ALL values over time (with purge/merge options)
  }

  // Generic form of DataNodeModule (should never be used; trigger error!)
  template <typename VAL_TYPE, typename... MODS> class DataNodeModule {
  public:  DataNodeModule() { emp_assert(false, "Unknown module used in DataNode!"); }
  };

  // Base form of DataNodeModule (available in ALL data nodes.)
  template <typename VAL_TYPE>
  class DataNodeModule<VAL_TYPE> {
  protected:
    size_t val_count;
  public:
    DataNodeModule() : val_count(0) { ; }

    using value_t = VAL_TYPE;

    size_t GetCount() const { return val_count; }

    void AddDatum(const VAL_TYPE & val) { val_count++; }

    void Reset() { val_count = 0; }
  };

  // Specialized forms of DataNodeModule
  // == data::Current ==
  template <typename VAL_TYPE, typename... MODS>
  class DataNodeModule<VAL_TYPE, data::Current, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    VAL_TYPE cur_val;

    using this_t = DataNodeModule<VAL_TYPE, data::Current, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;
  public:
    DataNodeModule() { ; }

    const VAL_TYPE & GetCurrent() const { return cur_val; }

    void AddDatum(const VAL_TYPE & val) { cur_val = val; parent_t::AddDatum(val); }
  };

  template <typename VAL_TYPE, typename... MODS>
  class DataNodeModule<VAL_TYPE, data::Range, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    double total;
    double min;
    double max;

    using this_t = DataNodeModule<VAL_TYPE, data::Range, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
  public:
    DataNodeModule() : total(0.0), min(0), max(0) { ; }

    double GetTotal() const { return total; }
    double GetMean() const { return total / (double) base_t::val_count; }
    double GetMin() const { return min; }
    double GetMax() const { return max; }

    void AddDatum(const VAL_TYPE & val) {
      total += val;
      if (!val_count || val < min) min = val;
      if (!val_count || val > max) max = val;
      parent_t::AddDatum(val);
    }

    void Reset() {
      total = 0.0;
      min = 0.0;
      max = 0.0;
      parent_t::Reset();
    }
  };

  template <typename VAL_TYPE, typename... MODS>
  class DataNode : public DataNodeModule<VAL_TYPE, MODS...> {
  private:
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;

  public:

    // Methods to retrieve new data.
    inline void Add() { ; }

    template <typename... Ts>
    inline void Add(const VAL_TYPE & val, const Ts &... extras) {
      parent_t::AddDatum(val); Add(extras...);
    }

    // Methods to reset data.
    void Reset() { parent_t::Reset(); }
  };

}

#endif
