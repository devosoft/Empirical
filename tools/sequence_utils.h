// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_SEQUENCE_UTILS_H
#define EMP_SEQUENCE_UTILS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  These functions focus on dealing with generic sequence types.
//

#include "functions.h"

namespace emp {

  // --- Distance functions for any array-type objects ---

  // Hamming distance is a simple count of substitutions needed to convert on array to another.
  template <typename TYPE>
  int calc_hamming_distance(const TYPE & in1, const TYPE & in2, int offset=0) {
    if (offset < 0) return calc_hamming_distance(in2, in1, -offset);

    const auto size1 = in1.size();
    const auto size2 = in2.size();

    // Calculate by how much the strings overlap.
    int overlap = std::min( size1 - offset,  size2 );

    // Initialize the distance to that part of the strings which do not overlap.
    int num_diffs = size1 + size2 - 2 * overlap;

    // Step through the overlapped section and add on additional differences.
    for (size_t i = 0; i < overlap; i++) {
      if (in1[i + offset] != in2[i]) num_diffs++;
    }
    
    return num_diffs;
  }

  // Edit distance is the minimum number of insertions, deletions and substitutions to convert
  // one array to another.
  template <typename TYPE>
  int calc_edit_distance(const TYPE & in1, const TYPE & in2) {
    const auto size1 = in1.size();
    const auto size2 = in2.size();

    // If either size is zero, other size indicates number of insertions needed to produce it.
    if (size1 == 0) return size2;
    if (size2 == 0) return size1;

    std::vector<int> cur_row(size1);   // The row we are calculating
    std::vector<int> prev_row(size1);  // The previous row we calculated

    // Initialize the previous row to record the differece from nothing.
    for (int i = 0; i < size1; i++) prev_row[i] = i + 1;

    // Loop through all other rows
    for (int row = 0; row < (int) size2; row++) {
      // Initialize the first entry in the current row.
      if (in1[0] == in2[row]) cur_row[0] = row;
      else cur_row[0] = std::min(row, prev_row[0]) + 1;

      // Move through the cur_row and fill it in.
      for (int col = 1; col < (int) size1; col++) {
        // If the values are equal, keep the value in the upper left.
        if (in1[col] == in2[row]) cur_row[col] = prev_row[col-1];

        // Otherwise, set the current position the the minimal of the three
        // numbers to the upper right in the chart plus one.
        else {
          cur_row[col] = emp::min(prev_row[col], prev_row[col-1], cur_row[col-1]) + 1;
        }
      }

      // Swap cur_row and prev_row (keep cur vals in prev row, recycle vector cur_row)
      std::swap(cur_row, prev_row);
    }

    // Now that we are done, return the bottom-right corner of the chart.
    return prev_row[size1 - 1];
  }

  // Use edit distance to find the minimum number of insertions, deletions and substitutions
  // to convert one array to another, and then insert gaps into the arrays appropriately.
  template <typename TYPE, typename GAP_TYPE>
  int align(TYPE & in1, TYPE & in2, GAP_TYPE gap) {
    const auto size1 = in1.size();
    const auto size2 = in2.size();

    // If either size is zero, other size indicates number of insertions needed to produce it.
    if (size1 == 0) return size2;
    if (size2 == 0) return size1;

    std::vector<int> cur_row(size1);   // The row we are calculating
    std::vector<int> prev_row(size1);  // The previous row we calculated
    std::vector<std::vector<char> > edit_info(size2, std::vector<char>(size1));

    // Initialize the previous row to record the differece from nothing.
    for (int i = 0; i < size1; i++) {
      prev_row[i] = i + 1;
      edit_info[0][i] = 'i';
    }

    // Loop through all other rows
    for (int row = 0; row < (int) size2; row++) {
      // Initialize the first entry in the current row.
      if (in1[0] == in2[row]) { cur_row[0] = row; edit_info[row][0] = 's'; }
      else { cur_row[0] = prev_row[0] + 1; edit_info[row][0] = 'd'; }
      
      // Move through the cur_row and fill it in.
      for (int col = 1; col < (int) size1; col++) {
        // If the values are equal, keep the value in the upper left.
        if (in1[col] == in2[row]) { cur_row[col] = prev_row[col-1]; edit_info[row][col] = 's'; }

        // Otherwise, set the current position to the minimum of the three
        // numbers to the left, upper, or upper left in the chart plus one.
        else {
          cur_row[col] = emp::min(prev_row[col], prev_row[col-1], cur_row[col-1]) + 1;
          if (cur_row[col] == prev_row[col]+1)   { edit_info[row][col] = 'd'; }
          if (cur_row[col] == prev_row[col-1]+1) { edit_info[row][col] = 's'; }
          if (cur_row[col] == cur_row[col-1]+1)  { edit_info[row][col] = 'i'; }
        }
      }

      // Swap cur_row and prev_row (keep cur vals in prev row, recycle vector cur_row)
      std::swap(cur_row, prev_row);
    }

    // Fill in gaps in the sequences to make them align!
    
    int c = size1-1;
    int r = size2-1;
    int length = 0;

    while (c >= 0 || r >= 0) {
      if (c < 0) { ++length; --r; continue; }
      else if (r < 0) { ++length; --c; continue; }

      char cur_move = edit_info[r][c];
      switch(cur_move) {
      case 's': --c; --r; ++length; break;
      case 'd': --r; ++length; break;
      case 'i': --c; ++length; break;
      };
    }
      
    c = size1-1;
    r = size2-1;

    TYPE out1(length, gap);
    TYPE out2(length, gap);

    int pos = length - 1;

    while (c >= 0 && r >= 0) {
      switch(edit_info[r][c]) {
      case 's': out1[pos] = in1[c]; out2[pos] = in2[r]; --c; --r; break;
      case 'd': out1[pos] = gap;    out2[pos] = in2[r];      --r; break;
      case 'i': out1[pos] = in1[c]; out2[pos] = gap;    --c;      break;
      };
      --pos;
    }
    while (c >= 0) { out1[pos] = in1[c]; --c; --pos; }
    while (r >= 0) { out2[pos] = in2[r]; --r; --pos; }

    in1 = out1;
    in2 = out2;

    // Now that we are done, return the bottom-right corner of the chart.
    return prev_row[size1 - 1];
  }

}

#endif
