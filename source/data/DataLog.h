/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  DataLog.h
 *  @brief Tools for processing a single set of data.
 *  @note  Status: ALPHA
 *
 */

#ifndef EMP_DATA_LOG_H
#define EMP_DATA_LOG_H

#include <iostream>
#include <ostream>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/vector_utils.h"

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

    T Min() { emp::FindMin(data); }
    T Max() { emp::FindMax(data); }
    T Mean() { emp::CalcMean(data); }

    /// The following function prints an ascii bar graph on to the screen (or provided stream).
    void AsciiBarGraph( size_t max_width=80,          ///< What's the widest bars allowed?
                        bool show_scale=true,         ///< Should we show the scale at bottom.
                        bool max_scale_1=true,        ///< Should we limit scaling to 1:1?
                        std::ostream & os=std::cout)  ///< Where to output the bar graph?
    {
      T min_size = emp::FindMin(data);
      T max_size = emp::FindMax(data);
      double scale = ((double) max_width) / ((double) max_size);
      if (max_scale_1 && scale > 1.0) scale = 1.0;
      for (T datum : data) {
        double bar_width = datum * scale;
        while (bar_width >= 1.0) { os << '='; bar_width -= 1.0; }
        if (bar_width > 0.0) os << '~';
        os << "  (" << datum << ")\n";
      }
      if (show_scale) {
        os << "SCALE: = -> " << (1.0 / scale) << std::endl;
      }
    }


    /// Take the input data, break it into bins, and print it as a bar graph.
    void AsciiHistogram(size_t num_bins=40,            ///< How many bins in histogram?
                        size_t max_width=80,           ///< What's the widest bars allowed?
                        bool show_scale=true,          ///< Should we show the scale at bottom?
                        std::ostream & os=std::cout)   ///< Where to output the bar graph?
    {
      T min_val = emp::FindMin(data);
      T max_val = emp::FindMax(data);
      T val_range = max_val - min_val;
      T bin_width = val_range / (T) num_bins;

      emp::vector<size_t> bins(num_bins, 0);
      for (T d : data) {
        size_t bin_id = (size_t) ( (d - min_val) / bin_width );
        if (bin_id == num_bins) bin_id--;
        bins[bin_id]++;
      }

      DataLog<size_t>(bins).AsciiBarGraph(max_width, show_scale, true, os);
    }
  };   
}

#endif
