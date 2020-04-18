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
    T min_size = emp::FindMin(bars);
    T max_size = emp::FindMax(bars);
    double scale = ((double) max_size) / (double) max_width;
    if (max_scale_1 && scale > 1.0) scale = 1.0;
    for (T datum : data) {
      double bar_width = datum * scale;
      while (bar_width >= 1.0) { os << '*'; bar_width -= 1.0; }
      if (bar_width > 0.0) os << '.';			     
      os << '\n';
    }
  }
  
}

#endif
