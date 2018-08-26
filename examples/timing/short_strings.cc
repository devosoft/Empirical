//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Code comparing various methods of accessing entries in an unsorted_map.
//
//
//  We are comparing the timings for accessing an unsorted map using:
//  * Numerical IDs
//  * Short strings
//  * Longer strings
//  * Floating point IDs

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <iostream>      // For std::cout, etc.
#include <string>
#include <unordered_map>

constexpr size_t NUM_ENTRIES = 1000000;
constexpr size_t LONG_STR_SIZE = 40;

std::string ToStringID(int id) { return std::to_string(id); }
std::string ToLongStringID(int id) {
  std::string out_str = ToStringID(id);
  out_str += std::string('*', LONG_STR_SIZE - out_str.size() );
  return out_str;
}
double ToFloatID(int id) { return 0.5 + (double) id; }

int main()
{
  // Create the maps.
  std::unordered_map<int, int> int_ids;
  std::unordered_map<std::string, int> short_strings;
  std::unordered_map<std::string, int> long_strings;
  std::unordered_map<double, int> float_ids;

  // Fill out the maps.
  for (int i = 0; i < NUM_ENTRIES; i++) {
    int_ids[i] = i;
    short_strings[ToStringID(i)] = i;
    long_strings[ToLongStringID(i)] = i;
    float_ids[ToFloatID(i)] = i;
  }

  std::clock_t base_start_time = std::clock();

  ;

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << "base sum = " << 0
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;
}
