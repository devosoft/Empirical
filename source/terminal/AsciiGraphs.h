/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  ascii_graphs.h
 *  @brief tools for making quick graphs that print to the terminal or files.
 *  @note Status: ALPHA
 *
 */

#ifndef ASCII_GRAPHS_H
#define ASCII_GRAPHS_H

#include <iostream>
#include <ostream>

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/vector_utils.h"

namespace emp {

  /// The following function prints an ascii bar graph on to the screen (or provided stream).
  template <typename T>
  void AsciiBarGraph(const emp::vector<T> & data,     ///< Data for the bar graph
		     size_t max_width=80,             ///< What's the widest bars allowed?
		     bool max_scale_1=true,           ///< Should we limit scaling to 1:1?
		     std::ostream & os=std::cout) {
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
  }


  /// Take the input data, break it into bins, and print it as a bar graph.
  template <typename T>
  void AsciiHistogram(const emp::vector<T> & data,    ///< Data to bin for histogram
		     size_t num_bins=40,              ///< How many bins in histogram?
		     size_t max_width=80,             ///< What's the widest bars allowed?
		     std::ostream & os=std::cout) {
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

    AsciiBarGraph(bins, max_width, true, os);
  }
		     
}

#endif
