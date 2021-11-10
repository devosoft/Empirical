/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file DataLog.hpp
 *  @brief Tools for processing a single set of data.
 *  @note  Status: ALPHA
 *
 */

#ifndef EMP_DATA_DATALOG_HPP_INCLUDE
#define EMP_DATA_DATALOG_HPP_INCLUDE

#include <iostream>
#include <ostream>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../io/ascii_utils.hpp"
#include "../math/stats.hpp"

namespace emp {

  template <typename T>
  class DataLog {
  private:
    emp::vector<T> data;

    using data_t = T;

  public:
    DataLog() = default;
    DataLog(const DataLog &) = default;
    DataLog(DataLog &&) = default;
    DataLog(const emp::vector<T> & in_data) : data(in_data) { }

    DataLog & operator=(const DataLog &) = default;
    DataLog & operator=(DataLog &&) = default;
    DataLog & operator=(const emp::vector<T> & in_data) { data = in_data; }

    size_t GetSize() { return data.size(); }
    size_t size() { return data.size(); }

    T & operator[](size_t id) { return data[id]; }
    const T & operator[](size_t id) const { return data [id]; }

    DataLog & Push(T in) { data.push_back(in); return *this; }
    DataLog & Sort() { emp::Sort(data); return *this; }

    T Min() const { emp::FindMin(data); }
    T Max() const { emp::FindMax(data); }
    T Mean() const { emp::Mean(data); }
    T Variance() const { emp::Variance(data); }
    T StdDev() const { emp::StandardDeviation(data); }
    T StdError() const { emp::StandardError(data); }

    /// The following function prints an ascii bar graph on to the screen (or provided stream).
    void AsciiBarGraph( size_t max_width=80,          ///< What's the widest bars allowed?
                        bool show_scale=true,         ///< Should we show the scale at bottom.
                        bool max_scale_1=true,        ///< Should we limit scaling to 1:1?
                        std::ostream & os=std::cout)  ///< Where to output the bar graph?
    {
      emp::AsciiBarGraph(data, max_width, show_scale, max_scale_1, os);
    }


    /// Take the input data, break it into bins, and print it as a bar graph.
    void AsciiHistogram(size_t num_bins=40,            ///< How many bins in histogram?
                        size_t max_width=80,           ///< What's the widest bars allowed?
                        bool show_scale=true,          ///< Should we show the scale at bottom?
                        std::ostream & os=std::cout)   ///< Where to output the bar graph?
    {
      emp::AsciiHistogram(data, num_bins, max_width, show_scale, os);
    }
  };
}

#endif // #ifndef EMP_DATA_DATALOG_HPP_INCLUDE
