//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
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
#include <functional>    // For std::function
#include <string>
#include <unordered_map>

constexpr size_t NUM_ENTRIES = 1000000;
constexpr size_t LONG_STR_SIZE = 40;
constexpr size_t EVAL_STEPS = 20000000;

std::string ToStringID(int id) { return std::to_string(id); }
std::string ToLongStringID(int id) {
  std::string out_str = ToStringID(id);
  out_str += std::string('*', LONG_STR_SIZE - out_str.size() );
  return out_str;
}
double ToFloatID(int id) { return 0.5 + (double) id; }

void TimeFun(const std::string & name, std::function<int()> fun) {
  std::clock_t base_start_time = std::clock();

  int result = fun();

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << name
            << " result = " << result
            << "   time = " << ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " seconds." << std::endl;
}

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

  std::cout << "Starting tests!" << std::endl;

  TimeFun("Numerical IDs", [&int_ids]() {
    int id1 = 42, id2 = 100, id3 = 1000;
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      int_ids[id1] += int_ids[id2];
      int_ids[id3] -= int_ids[id2];
      int_ids[id2] = int_ids[id3] / 2 + 1000;
    }
    return int_ids[id1];
  });

  TimeFun("Short-string IDs", [&short_strings]() {
    std::string id1 = "42", id2 = "100", id3 = "1000";
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      short_strings[id1] += short_strings[id2];
      short_strings[id3] -= short_strings[id2];
      short_strings[id2] = short_strings[id3] / 2 + 1000;
    }
    return short_strings[id1];
  });

  TimeFun("Long-string IDs", [&long_strings]() {
    std::string id1 = ToLongStringID(42), id2 = ToLongStringID(100), id3 = ToLongStringID(1000);
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      long_strings[id1] += long_strings[id2];
      long_strings[id3] -= long_strings[id2];
      long_strings[id2] = long_strings[id3] / 2 + 1000;
    }
    return long_strings[id1];
  });

  TimeFun("Float IDs", [&float_ids]() {
    double id1 = ToFloatID(42), id2 = ToFloatID(100), id3 = ToFloatID(1000);
    for (size_t i = 0; i < EVAL_STEPS; i++) {
      float_ids[id1] += float_ids[id2];
      float_ids[id3] -= float_ids[id2];
      float_ids[id2] = float_ids[id3] / 2 + 1000;
    }
    return float_ids[id1];
  });
}
