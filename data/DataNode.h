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
//  Process: What should happen on Reset() ?
//   * Trigger an action to process the prior update's data stored.
//   * Clear all data.
//   * Send data to a stream
//     (or stats automatically have a stream that, if non-null data is sent to?)

#ifndef EMP_DATA_NODE_H
#define EMP_DATA_NODE_H

#include "../base/vector.h"
#include "../meta/IntPack.h"
#include "../tools/assert.h"
#include "../tools/FunctionSet.h"

namespace emp {

  // A set of modifiers are available do describe DataNode
  enum class data {
    Current,      // Track most recent value

    Log,          // Track all values since last Reset()
    // Archive,      // Track Log + ALL values over time (with purge options)

    Range,        // Track min, max, mean, total
    // Stats,        // Track Range + variance, standard deviation, skew, kertosis
    // FullStats,    // Track States + ALL values over time (with purge/merge options)

    Pull,         // Enable data collection on request.

    // Various signals are possible:
    SignalReset,  // Include a signal that triggers BEFORE Reset() to process data.
    SignalData,   // Include a signal when new data is added (as a group)
    SignalDatum,  // Include a signal when each datum is added.
    SignalRange,  // Include a signal for data in a range.
    SignalLimits, // Include a signal for data OUTSIDE a range.

    UNKNOWN       // Unknown modifier; will trigger error.
  };

  // Generic form of DataNodeModule (should never be used; trigger error!)
  template <typename VAL_TYPE, emp::data... MODS> class DataNodeModule {
  public:
    DataNodeModule() { emp_assert(false, "Unknown module used in DataNode!"); }
  };

  // Base form of DataNodeModule (available in ALL data nodes.)
  template <typename VAL_TYPE>
  class DataNodeModule<VAL_TYPE> {
  protected:
    size_t val_count;               // How many values have been loaded?
    emp::vector<VAL_TYPE> in_vals;  // What values are waiting to be included?

    void PullData_impl() { ; }
  public:
    DataNodeModule() : val_count(0) { ; }

    using value_t = VAL_TYPE;

    size_t GetCount() const { return val_count; }

    void AddDatum(const VAL_TYPE & val) { val_count++; }

    void Reset() { val_count = 0; }

    void PrintDebug(std::ostream & os=std::cout) {
      os << "BASE DataNodeModule.\n";
    }
  };

  // Specialized forms of DataNodeModule

  // == data::Current ==
  template <typename VAL_TYPE, emp::data... MODS>
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

    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Current. (level " << (int) data::Current << ")\n";
      parent_t::PrintDebug(os);
    }
  };


  // == data::Log ==
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Log, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    emp::vector<VAL_TYPE> val_set;

    using this_t = DataNodeModule<VAL_TYPE, data::Log, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
  public:
    DataNodeModule() { ; }

    void AddDatum(const VAL_TYPE & val) {
      val_set.push_back(val);
      parent_t::AddDatum(val);
    }

    void Reset() {
      val_set.resize(0);
      parent_t::Reset();
    }

    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Log. (level " << (int) data::Log << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  // == data::Range ==
  template <typename VAL_TYPE, emp::data... MODS>
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

    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Range. (level " << (int) data::Range << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  // == data::Pull ==
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Pull, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    emp::FunctionSet<VAL_TYPE()> pull_funs;
    emp::FunctionSet<emp::vector<VAL_TYPE>()> pull_set_funs;

    using this_t = DataNodeModule<VAL_TYPE, data::Pull, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::in_vals;

    void PullData_impl() {
      in_vals = pull_funs.Run();
      const emp::vector< emp::vector<VAL_TYPE> > & pull_sets = pull_set_funs.Run();
      for (const auto & x : pull_sets) {
        in_vals.insert(in_vals.end(), x.begin(), x.end());
      }
    }

  public:
    void AddPull(const std::function<VAL_TYPE()> & fun) { pull_funs.Add(fun); }
    void AddPullSet(const std::function<emp::vector<VAL_TYPE>()> & fun) { pull_set_funs.Add(fun); }

    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Pull. (level " << (int) data::Pull << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  template <typename VAL_TYPE, typename MOD_PACK> class DataNode_Interface;

  template <typename VAL_TYPE, int... IMODS>
  class DataNode_Interface<VAL_TYPE, IntPack<IMODS...>>
    : public DataNodeModule<VAL_TYPE, (emp::data) IMODS...> {
    using parent_t = DataNodeModule<VAL_TYPE, (emp::data) IMODS...>;
  };

  template <typename VAL_TYPE, emp::data... MODS>
  class DataNode : public DataNode_Interface< VAL_TYPE, pack::RUsort<IntPack<(int) MODS...>> > {
  private:
    using parent_t = DataNode_Interface< VAL_TYPE, pack::RUsort<IntPack<(int) MODS...>> >;
    using parent_t::in_vals;
    using test = IntPack<(int)MODS...>;

  public:

    // Methods to retrieve new data.
    inline void Add() { ; }

    template <typename... Ts>
    inline void Add(const VAL_TYPE & val, const Ts &... extras) {
      parent_t::AddDatum(val); Add(extras...);
    }

    void PullData() {
      parent_t::PullData_impl();                                     // Pull all data into in_vals.
      for (const VAL_TYPE & val : in_vals) parent_t::AddDatum(val);  // Actually add the data.
    }

    // Methods to reset data.
    void Reset() { parent_t::Reset(); }

    // Methods to debug.
    void PrintDebug(std::ostream & os=std::cout) {
      os << "Main DataNode.\n";
      parent_t::PrintDebug(os);
    }
  };

}

#endif
