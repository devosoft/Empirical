/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  DataNode.h
 *  @brief DataNode objects track a specific type of data over the course of a run.
 *
 *  Collection: New data can be pushed or pulled.
 *   Add(VAL... v) pushes data to a node
 *   AddDatum(VAL v) pushes just one datum, but can be used as an action for a signal.
 *
 *  Process: What should happen on Reset() ?
 *   * Trigger an action to process the prior update's data stored.
 *   * Clear all data.
 *   * Send data to a stream
 *     (or stats automatically have a stream that, if non-null data is sent to?)
 *
 *  @todo: The Archive data node should have Log as a requiste and then copy the current vals into the
 *         archive on reset.  This change will also make it so that the size of the archive correctly
 *         reflects the number of resets.
 */

#ifndef EMP_DATA_NODE_H
#define EMP_DATA_NODE_H

#include <limits>
#include <algorithm>

#include "../base/vector.h"
#include "../base/assert.h"
#include "../meta/ValPack.h"
#include "../tools/FunctionSet.h"
#include "../tools/IndexMap.h"
#include "../tools/string_utils.h"
#include "../tools/math.h"

namespace emp {

  /// A set of modifiers are available do describe DataNode
  enum class data {
    Current,      ///< Track most recent value

    Info,         ///< Include information (name, keyword, description) for each instance.

    Log,          ///< Track all values since last Reset()
    Archive,      ///< Track Log + ALL values over time (with purge options)

    Range,        ///< Track min, max, mean, total
    FullRange,    ///< Track Range data over time.
    Histogram,    ///< Keep a full histogram.
    Stats,        ///< Track Range + variance, standard deviation, skew, kertosis
    // FullStats,    // Track States + ALL values over time (with purge/merge options)

    Pull,         ///< Enable data collection on request.

    ///< Various signals are possible:
    SignalReset,  ///< Include a signal that triggers BEFORE Reset() to process data.
    SignalData,   ///< Include a signal when new data is added (as a group)
    SignalDatum,  ///< Include a signal when each datum is added.
    SignalRange,  ///< Include a signal for data in a range.
    SignalLimits, ///< Include a signal for data OUTSIDE a range.

    UNKNOWN       ///< Unknown modifier; will trigger error.
  };


  /// A shortcut for converting DataNode mod ID's to ValPacks.
  template <emp::data... MODS> using ModPack = emp::ValPack<(int) MODS...>;

  /// Extra info about data modules that we need to know before actually building this DataNode.
  /// (for now, just REQUISITES for each module.)
  template <emp::data MOD> struct DataModInfo     { using reqs = ModPack<>; };
  template <> struct DataModInfo<data::Archive>   { using reqs = ModPack<data::Log>; };
  template <> struct DataModInfo<data::FullRange> { using reqs = ModPack<data::Range>; };
  template <> struct DataModInfo<data::Stats>     { using reqs = ModPack<data::Range>; };
  //template <> struct DataModInfo<data::FullStats> { using reqs = ModPack<data::Range, data::Stats>; };


  // A set of structs to collect and merge data module requisites.
  template <emp::data... MODS> struct DataModuleRequisiteAdd { };
  template <> struct DataModuleRequisiteAdd<> { using type = ValPack<>; };
  template <emp::data CUR_MOD, emp::data... MODS> struct DataModuleRequisiteAdd<CUR_MOD, MODS...> {
    using next_type = typename DataModuleRequisiteAdd<MODS...>::type;
    using this_req = typename DataModInfo<CUR_MOD>::reqs;
    using type = typename next_type::template append<this_req>;
  };


  /// Generic form of DataNodeModule (should never be used; trigger error!)
  template <typename VAL_TYPE, emp::data... MODS> class DataNodeModule {
  public:
    DataNodeModule() { emp_assert(false, "Unknown module used in DataNode!"); }
  };

  /// Base form of DataNodeModule (available in ALL data nodes.)
  template <typename VAL_TYPE>
  class DataNodeModule<VAL_TYPE> {
  protected:
    size_t val_count;               ///< How many values have been loaded?
    emp::vector<VAL_TYPE> in_vals;  ///< What values are waiting to be included?

    void PullData_impl() { ; }

  public:
    DataNodeModule() : val_count(0), in_vals() { ; }

    using value_t = VAL_TYPE;

    /// Return the number of values that have been added to this node since the last reset
    size_t GetCount() const { return val_count; }

    /// If reset count not tracked, always return 0.  If any modifier causes history to be saved, it will
    /// override this function and indicate how much history is stored.
    size_t GetResetCount() const { return 0; }

    double GetTotal() const {emp_assert(false, "Calculating total requires a DataNode with the Range or FullRange modifier"); return 0;}
    double GetMean() const {emp_assert(false, "Calculating mean requires a DataNode with the Range or FullRange modifier"); return 0;}
    double GetMin() const {emp_assert(false, "Calculating min requires a DataNode with the Range or FullRange modifier"); return 0;}
    double GetMax() const {emp_assert(false, "Calculating max requires a DataNode with the Range or FullRange modifier"); return 0;}
    double GetVariance() const {emp_assert(false, "Calculating variance requires a DataNode with the Stats or FullStats modifier"); return 0;}
    double GetStandardDeviation() const {emp_assert(false, "Calculating standard deviation requires a DataNode with the Stats or FullStats modifier"); return 0;}
    double GetSkew() const {emp_assert(false, "Calculating skew requires a DataNode with the Stats or FullStats modifier"); return 0;}
    double GetKurtosis() const {emp_assert(false, "Calculating kurtosis requires a DataNode with the Stats or FullStats modifier"); return 0;}

    /// Calculate the median of observed values
    double GetMedian() const {emp_assert(false, "Calculating median requires a DataNode with the Log modifier"); return 0;}
    /// Calculate a percentile of observed values
    double GetPercentile(const double pct) const {emp_assert(false, "Calculating percentile requires a DataNode with the Log modifier"); return 0;}

    const std::string & GetName() const { return emp::empty_string(); }
    const std::string & GetDescription() const { return emp::empty_string(); }
    const std::string & GetKeyword() const { return emp::empty_string(); }

    void SetName(const std::string &) { emp_assert(false, "Invalid call for DataNode config."); }
    void SetDescription(const std::string &) { emp_assert(false, "Invalid call for DataNode config."); }
    void SetKeyword(const std::string &) { emp_assert(false, "Invalid call for DataNode config."); }

    void SetInfo(const std::string &, const std::string & _d="", const std::string & _k="") {
      (void) _d; (void) _k;
      emp_assert(false, "Invalid call for DataNode config.");
    }

    void AddDatum(const VAL_TYPE & val) { val_count++; }

    void Reset() { val_count = 0; }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "BASE DataNodeModule.\n";
    }
  };

  // Specialized forms of DataNodeModule

  /// == data::Current ==
  /// This module lets you track the current (i.e. most recently added) value
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Current, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    VAL_TYPE cur_val;  ///< Most recent value passed to this node.

    using this_t = DataNodeModule<VAL_TYPE, data::Current, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

  public:
    DataNodeModule() : cur_val() { ; }

    /// Return the current (most recently added) value
    const VAL_TYPE & GetCurrent() const { return cur_val; }

    /// Add @param val to this DataNode
    void AddDatum(const VAL_TYPE & val) { cur_val = val; parent_t::AddDatum(val); }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Current. (level " << (int) data::Current << ")\n";
      parent_t::PrintDebug(os);
    }
  };


  /// == data::Info ==
  /// This module adds information such as a name, description, and keyword for this node.
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Info, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    std::string name;     ///< Name of this data category.
    std::string desc;     ///< Description of this type of data.
    std::string keyword;  ///< Short keyword.

    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
  public:
    DataNodeModule() : name(), desc(), keyword() { ; }

    /// Get this DataNode's name
    const std::string & GetName() const { return name; }
    /// Get this DataNode's description
    const std::string & GetDescription() const { return desc; }
    /// Get this DataNode's keyword
    const std::string & GetKeyword() const { return keyword; }

    /// Set this DataNode's name to @param _in
    void SetName(const std::string & _in) { name = _in; }
    /// Set this DataNode's description to @param _in
    void SetDescription(const std::string & _in) { desc = _in; }
    /// Set this DataNode's keyword to @param _in
    void SetKeyword(const std::string & _in) { keyword = _in; }

    /// Set this DataNode's name to @param _n, description to @param _d, and keyword to @param _k
    void SetInfo(const std::string & _n, const std::string & _d="", const std::string & _k="") {
      name = _n;  desc = _d;  keyword = _k;
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Info. (level " << (int) data::Info << ")\n";
      parent_t::PrintDebug(os);
    }
  };


  /// == data::Log ==
  /// This module lets you log all of the values that have been added since the last re-set
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Log, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    emp::vector<VAL_TYPE> val_set;  ///< All values saved since last reset.

    using this_t = DataNodeModule<VAL_TYPE, data::Log, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
  public:
    DataNodeModule() : val_set() { ; }

    /// Get a vector of all data added since the last reset
    const emp::vector<VAL_TYPE> & GetData() const { return val_set; }

    /// Calculate the median of observed values
    double GetMedian() const {
      return GetPercentile(50);
    }

    /// Calculate a percentile of observed values
    double GetPercentile(const double pct) const {

      emp_assert(pct >= 0.0);
      emp_assert(pct <= 100.0);

      if (!val_set.size()) return std::numeric_limits<double>::quiet_NaN();

      emp::vector<VAL_TYPE> dup(std::begin(val_set), std::end(val_set));
      std::sort(std::begin(dup), std::end(dup));
      const double idx = pct/100.0 * (dup.size() - 1);

      // if needed, linearly interpolate
      return dup[idx] == dup[std::ceil(idx)]
        ? dup[idx]
        : (
          dup[idx] * (1.0 - emp::Mod(idx, 1.0))
          + dup[std::ceil(idx)] * emp::Mod(idx, 1.0)
        );
    }

    /// Add @param val to this DataNode
    void AddDatum(const VAL_TYPE & val) {
      val_set.push_back(val);
      parent_t::AddDatum(val);
    }

    /// Reset this DataNode (clear the current log of data)
    void Reset() {
      val_set.resize(0);
      parent_t::Reset();
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Log. (level " << (int) data::Log << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  /// == data::Archive ==
  /// This module keeps track of historical values in addition to those added since the last re-set.
  /// Every time Reset() is called, all values that have been added since the previous time Reset()
  /// are stored in a vector in the archive.
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Archive, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    emp::vector<emp::vector<VAL_TYPE>> archive;  ///< Data archived from before most recent reset.

    using this_t = DataNodeModule<VAL_TYPE, data::Archive, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
    using parent_t::val_set;
  public:
    DataNodeModule() : archive(0) { ; }

    /// Get all data ever added to this DataNode. Returns a vector of vectors; each vector
    /// contains all data from a single time point (interval between resets)
    const auto & GetArchive() const { return archive; }

    /// Get a vector of all data that was added during the @param update 'th interval between resets.
    const emp::vector<VAL_TYPE> & GetData(size_t update) const { return archive[update]; }

    /// Get a vector of all data that has been added since the last reset
    const emp::vector<VAL_TYPE> & GetData() const { return val_set; }

    /// Get the number of time intervals recorded in this DataNode.
    /// Note that this is one more than the number of times it has been reset
    size_t GetResetCount() const { return archive.size(); }

    // NOTE: Ignoring AddDatum() since new value will be added to val_set.

    /// Reset this DataNode, starting a new grouping of values in the archive.  Resetting is
    /// useful for tracking data from different time points, such as per update or generation.
    void Reset() {
      archive.push_back(val_set);
      parent_t::Reset();
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Archive. (level " << (int) data::Archive << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  /// == data::Range ==
  /// This module allows this DataNode to store information (min, max, mean, count, and total) about
  /// the distribution of the values that have been added since the last call to Reset().
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Range, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    double total;  ///< Total of all data since last reset.
    double min;    ///< Smallest value passed in since last reset.
    double max;    ///< Largest value passed in since last reset.

    using this_t = DataNodeModule<VAL_TYPE, data::Range, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
  public:
    DataNodeModule() : total(0.0), min(0), max(0) { ; }

    /// Get the sum of all values added to this DataNode since the last reset
    double GetTotal() const { return total; }

    /// Get the mean of all values added to this DataNode since the last reset
    double GetMean() const { return total / (double) base_t::val_count; }

    /// Get the min of all values added to this DataNode since the last reset
    double GetMin() const { return min; }

    /// Get the max of all values added to this DataNode since the last reset
    double GetMax() const { return max; }

    /// Add @param val to this DataNode
    void AddDatum(const VAL_TYPE & val) {
      total += (double) val;
      if (!val_count || min > (double) val) min = (double) val;
      if (!val_count || max < (double) val) max = (double) val;
      parent_t::AddDatum(val);
    }

    /// Reset DataNode, setting the running calucluations of total, min, mean, and max to 0
    void Reset() {
      total = 0.0;
      min = 0.0;
      max = 0.0;
      parent_t::Reset();
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Range. (level " << (int) data::Range << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  /// == data::FullRange ==
  /// This module makes the DataNode store a history of distributional information measured by
  /// data::Range beteen calls to Reset().  Series of historical values are stored in vectors
  /// (except mean, which is calculated from total and count).
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::FullRange, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    emp::vector<double> total_vals;  ///< Totals from previous resets.
    emp::vector<size_t> num_vals;    ///< Value counts from previous resets.
    emp::vector<double> min_vals;    ///< Minimums from previous resets.
    emp::vector<double> max_vals;    ///< Maximums from previous resets.

    using this_t = DataNodeModule<VAL_TYPE, data::FullRange, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
    using parent_t::total;
    using parent_t::min;
    using parent_t::max;
  public:
    DataNodeModule()
      : total_vals(), num_vals(), min_vals(), max_vals() { ; }

    /// Get the sum of all values added to this DataNode since the last reset
    double GetTotal() const { return total; }

    /// Get the mean of all values added to this DataNode since the last reset
    double GetMean() const { return total / (double) val_count; }

    /// Get the minimum of all values added to this DataNode since the last reset
    double GetMin() const { return min; }

    /// Get the maximum of all values added to this DataNode since the last reset
    double GetMax() const { return max; }

    /// Get the sum of all values added to this DataNode during the @param update specified.
    double GetTotal(size_t update) const { return total_vals[update]; }

    /// Get the mean of all values added to this DataNode during the @param update specified.
    double GetMean(size_t update) const { return total_vals[update] / (double) num_vals[update]; }

    /// Get the minimum of all values added to this DataNode during the @param update specified.
    double GetMin(size_t update) const { return min_vals[update]; }

    /// Get the maximum of all values added to this DataNode during the @param update specified.
    double GetMax(size_t update) const { return max_vals[update]; }

    /// Get the number of time intervals recorded in this DataNode.
    ///  Note that this is one more than the number of times it has been reset
    size_t GetResetCount() const { return total_vals.size(); }

    // NOTE: Ignoring AddDatum() since Range values track current information.

    /// Store the current range statistics in the archive and reset for a new interval.
    void Reset() {
      total_vals.push_back(total);
      num_vals.push_back(val_count);
      min_vals.push_back(min);
      max_vals.push_back(max);
      parent_t::Reset();
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::FullRange. (level " << (int) data::FullRange << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  /// == data::Stats ==
  /// Note: These statistics are calculated with the assumption that the data this node has
  /// recieved is the entire population of measurements we're interested in, not a sample.
  ///
  /// Note 2: Kurtosis is calculated using Snedecor and Cochran (1967)'s formula. A perfect normal
  /// distribution has a kurtosis of 0.

  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Stats, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    // Running variance, skew, and kurtosis calculations based off of this class:
    // https://www.johndcook.com/blog/skewness_kurtosis/

    // We don't need the mean (M1) because it's already being tracked
    double M2;  ///< The second moment of the distribution
    double M3;  ///< The third moment of the distribution
    double M4;  ///< The fourth moment of the distribution

    using this_t = DataNodeModule<VAL_TYPE, data::Stats, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;
    using parent_t::total;
    using parent_t::min;
    using parent_t::max;

  public:
    DataNodeModule() : M2(0), M3(0), M4(0) { ; }

    using parent_t::GetMean;

    /// Get the variance (squared deviation from the mean) of values added since the last reset
    double GetVariance() const {return M2/(double)val_count;}

    /// Get the standard deviation of values added since the last reset
    double GetStandardDeviation() const {return sqrt(GetVariance());}

    /// Get the skewness of values added since the last reset. This measurement tells you about
    /// the shape of the distribution. For a unimodal distribution, negative skew means that the
    /// distribution has a longer/thicker tail to the left. Positive skew means that ths distribution
    /// has a longer/thicker tail to the right.
    double GetSkew() const {return sqrt(double(val_count)) * M3/ emp::Pow(M2, 1.5);}

    /// Get the kurtosis of the values added since the last reset. This is another measurement that
    /// describes the shape of the distribution. High kurtosis means that there is more data in the
    /// tails of the distribution (i.e. the tails are "heavier"), whereas low kurtosis means that
    /// there is less data in the tails. We use Snedecor and Cochran (1967)'s formula to calculate
    /// kurtosis. Under this formula, a normal distribution has kurtosis of 0.
    double GetKurtosis() const {return double(val_count)*M4 / (M2*M2) - 3.0;}

    /// Add @param val to this DataNode
    void AddDatum(const VAL_TYPE & val) {
      // Calculate deviation from mean (the ternary avoids dividing by
      // 0 in the case where this is the first datum added since last reset)
      const double n = (double) (val_count + 1);
      const double delta = ((double) val) - (total/((val_count > 0) ? (double) val_count : 1.0));
      const double delta_n = delta / n;
      const double delta_n2 = delta_n * delta_n;
      const double term1 = delta * delta_n * (double) val_count;

      M4 += term1 * delta_n2 * (n*n - 3.0*n + 3.0) + 6.0 * delta_n2 * M2 - 4.0 * delta_n * M3;
      M3 += term1 * delta_n * (n - 2.0) - 3.0 * delta_n * M2;
      M2 += term1;

      parent_t::AddDatum(val);
    }

    /// Reset this node (resets current stats to 0)
    void Reset() {
      M2 = 0;
      M3 = 0;
      M4 = 0;
      parent_t::Reset();
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Stats. (level " << (int) data::Stats << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  /// == data::Histogram ==
  /// Make the DataNode track a histogram of values observed since the last reset.
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Histogram, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    VAL_TYPE offset;              ///< Min value in first bin; others are offset by this much.
    VAL_TYPE width;               ///< How wide is the overall histogram?
    IndexMap bins;                ///< Map of values to which bin they fall in.
    emp::vector<size_t> counts;   ///< Counts in each bin.

    using this_t = DataNodeModule<VAL_TYPE, data::Histogram, MODS...>;
    using parent_t = DataNodeModule<VAL_TYPE, MODS...>;
    using base_t = DataNodeModule<VAL_TYPE>;

    using base_t::val_count;

  public:
    DataNodeModule() : offset(0.0), width(0), bins(), counts() { ; }

    /// Returns the minimum value this histogram is capable of containing
    /// (i.e. the minimum value for the first bin)
    VAL_TYPE GetHistMin() const { return offset; }

    /// Returns the maximum value this histogram is capable of containing
    /// (i.e. the maximum value for the last bin)
    VAL_TYPE GetHistMax() const { return offset + width; }

    /// Return the count of items in the @param bin_id 'th bin of the histogram
    size_t GetHistCount(size_t bin_id) const { return counts[bin_id]; }

    /// Return the width of the @param bin_id 'th bin of the histogram
    double GetHistWidth(size_t bin_id) const { return bins[bin_id]; } //width / (double) counts.size(); }

    /// Return a vector containing the count of items in each bin of the histogram
    const emp::vector<size_t> & GetHistCounts() const { return counts; }

    /// Return a vector containing the lowest value allowed in each bin.
    emp::vector<double> GetBinMins() const {
      emp::vector<double> bin_mins(counts.size());
      // double bin_width = width / (double) counts.size();
      double cur_min = offset;
      for (size_t i = 0; i < counts.size(); i++) {
        bin_mins[i] = cur_min;
        cur_min += bins[i]; // bin_width;
      }
      return bin_mins;
    }

    /// Sets up the ranges of values that go in each bin of the histogram.
    /// @param _min - the lowest value allowed in the histogram
    /// @param _max - the largest value allowed in the histogram
    /// @param num_bins - The number of bins the histogram should have. The distance
    ///                   between min and max will be easily divided among this many bins.
    void SetupBins(VAL_TYPE _min, VAL_TYPE _max, size_t num_bins) {
      offset = _min;
      width = _max - _min;
      double bin_width = ((double) width) / (double) num_bins;
      bins.Resize(num_bins);
      bins.AdjustAll(bin_width);
      counts.resize(num_bins);
      for (size_t & x : counts) x = 0.0;
    }

    /// Add @param val to the DataNode
    void AddDatum(const VAL_TYPE & val) {
      size_t bin_id = bins.Index((double) (val - offset));
      // size_t bin_id = counts.size() * ((double) (val - offset)) / (double) width;
      counts[bin_id]++;
      parent_t::AddDatum(val);
    }

    /// Reset the DataNode (empties the historgram)
    void Reset() {
      for (size_t & x : counts) x = 0.0;
      parent_t::Reset();
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Histogram. (level " << (int) data::FullRange << ")\n";
      parent_t::PrintDebug(os);
    }

  };

  /// == data::Pull ==
  /// This module makes it possible to give the DataNode a function that it can call to calculate
  /// new values or sets of values that it will then track. These functions are called every time
  /// the PullData method is called on this node, and the values they return are measured as
  /// specified by the other modules in this node.
  template <typename VAL_TYPE, emp::data... MODS>
  class DataNodeModule<VAL_TYPE, data::Pull, MODS...> : public DataNodeModule<VAL_TYPE, MODS...> {
  protected:
    emp::FunctionSet<VAL_TYPE()> pull_funs;                   ///< Functions to pull data.
    emp::FunctionSet<emp::vector<VAL_TYPE>()> pull_set_funs;  ///< Functions to pull sets of data.

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
    DataNodeModule() : pull_funs(), pull_set_funs() { ; }

    void AddPull(const std::function<VAL_TYPE()> & fun) { pull_funs.Add(fun); }
    void AddPullSet(const std::function<emp::vector<VAL_TYPE>()> & fun) { pull_set_funs.Add(fun); }

    void PrintDebug(std::ostream & os=std::cout) {
      os << "DataNodeModule for data::Pull. (level " << (int) data::Pull << ")\n";
      parent_t::PrintDebug(os);
    }
  };

  template <typename VAL_TYPE, typename MOD_PACK> class DataNode_Interface;

  /// Outermost interface to all DataNode modules.
  template <typename VAL_TYPE, int... IMODS>
  class DataNode_Interface<VAL_TYPE, ValPack<IMODS...>>
    : public DataNodeModule<VAL_TYPE, (emp::data) IMODS...> {
    using parent_t = DataNodeModule<VAL_TYPE, (emp::data) IMODS...>;
  };

  /// A template that will determing requisites, sort, make unique the data mods provided.
  /// The final, sorted ValPack of the requisites plus originals is in 'sorted'.
  template<emp::data... MODS>
  struct FormatDataMods {
    using reqs = typename DataModuleRequisiteAdd<MODS...>::type;    ///< Identify requisites
    using full = typename ModPack<MODS...>::template append<reqs>;  ///< Requisites + originals
    using sorted = pack::RUsort<full>;                              ///< Unique and in order
  };

  template <typename VAL_TYPE, emp::data... MODS>
  class DataNode : public DataNode_Interface< VAL_TYPE, typename FormatDataMods<MODS...>::sorted > {
  private:
    using parent_t = DataNode_Interface< VAL_TYPE, typename FormatDataMods<MODS...>::sorted  >;
    using parent_t::in_vals;

  public:

    inline void Add() { ; }

    /// Methods to provide new data.
    template <typename... Ts>
    inline void Add(const VAL_TYPE & val, const Ts &... extras) {
      parent_t::AddDatum(val); Add(extras...);
    }

    /// Method to retrieve new data.
    void PullData() {
      parent_t::PullData_impl();                                     // Pull all data into in_vals.
      for (const VAL_TYPE & val : in_vals) parent_t::AddDatum(val);  // Actually add the data.
    }

    /// Methods to reset data.
    void Reset() { parent_t::Reset(); }

    /// Methods to Print the templated values that a DataNode can produce.
    void PrintCurrent(std::ostream & os=std::cout) const { os << parent_t::GetCurrent(); }
    void PrintLog(std::ostream & os=std::cout,
                 const std::string & spacer=", ",
                 const std::string & eol="\n") const {
      const emp::vector<VAL_TYPE> & data = parent_t::GetData();
      for (size_t i=0; i < data.size(); i++) {
        if (i>0) os << spacer;
        os << data[i];
      }
      os << eol;
    }

    /// Print debug information (useful for figuring out which modifiers you included)
    void PrintDebug(std::ostream & os=std::cout) {
      os << "Main DataNode.\n";
      parent_t::PrintDebug(os);
    }
  };

  // Shortcuts for common types of data nodes...

  /** A node that stores data about the most recent value it recieved, as well as the
   * distribution (min, max, count, total, and mean) of values it has recieved since
   * the last reset. It also allows you to give it a name, description, and keyword.*/
  template <typename T, emp::data... MODS>
  using DataMonitor = DataNode<T, data::Current, data::Info, data::Range, data::Stats, MODS...>;

  /** A node that stores data about the most recent value it recieved, as well as all
   * values it has recieved since the last reset. It also allows you to give it a name,
   * description, and keyword.*/
  template <typename T, emp::data... MODS>
  using DataLog = DataNode<T, data::Current, data::Info, data::Log, MODS...>;

  /** A node that stores all data it recieves in an archive (vector of vectors). The inner
   * vectors are groups of data that were recieved between resets. This node also keeps
   * a record of the min, max, count, and total of each vector, so you don't have to
   * recalculate it later. Additionally, it allows you to give it a name, description,
   * and keyword.*/
  template <typename T, emp::data... MODS>
  using DataArchive = DataNode<T, data::Info, data::Archive, data::FullRange, MODS...>;
}

#endif
