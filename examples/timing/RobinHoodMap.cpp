#include <ctime>
#include <flat_map>
#include <iostream>
#include <map>
#include <unordered_map>

#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"
#include "emp/datastructs/RobinHoodMap.hpp"
#include "emp/io/ANSI.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/tools/String.hpp"
#include "emp/tools/string_utils.hpp"

#include "../../third-party/robin-hood-hashing/src/include/robin_hood.h"

static constexpr size_t COUNT = 10000000;
static constexpr size_t MAX_STEP = 10;

emp::vector<size_t> keys(COUNT);
emp::vector<double> values(COUNT);

struct Results {
  emp::String scope;
  emp::String name;
  double insert_time;
  double search_time;
  double traverse_time;
  double erase_time;
  size_t found_count;
  size_t key_total;
  double value_total;
  size_t erase_count;
  size_t start_size;
  size_t final_size;
};

template <typename MAP_T>
Results Test(emp::String scope, emp::String name) {
  MAP_T test_map;
  Results results;
  results.scope = scope;
  results.name = name;

  // Test inserts
  std::clock_t start_time = std::clock();
  for (size_t i = 0; i < values.size(); ++i) {
    test_map.insert({keys[i], values[i]});
  }
  std::clock_t tot_time = std::clock() - start_time;
  results.insert_time = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  results.start_size = test_map.size();

// std::cout << "Test: " << name << "\n"
//           << "  insert time = " << emp::MakeString(result, " seconds.").AsANSIBlue() << "\n"
//           << "  start size = "  << test_map.size() << std::endl;
//           << "  capacity = "    << test_map.capacity() << std::endl;

  // Test searching
  start_time = std::clock();
  size_t found_count = 0;
  for (size_t i = 0; i < values.size() * 2; ++i) {
    if (test_map.contains(i)) ++found_count;
  }
  tot_time = std::clock() - start_time;
  results.search_time = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  results.found_count = found_count;

  if constexpr (std::same_as<MAP_T, emp::RobinHoodMap<size_t, double>> ||
                std::same_as<MAP_T, emp::RobinHoodMap<size_t, double, true>>) {
    test_map.EvalHashQuality();
  }

  // Test traversal
  start_time = std::clock();
  size_t key_total = 0;
  double value_total = 0.0;
  for (auto [key, value] : test_map) {
    key_total += key;
    value_total += value;
  }
  tot_time = std::clock() - start_time;
  results.traverse_time = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  results.key_total = key_total;
  results.value_total = value_total;

  // Test deletion
  start_time = std::clock();
  size_t erase_count = 0;
  for (size_t i = values.size(); i >= 500; --i) {
    if (test_map.erase(i)) ++erase_count;
  }
  tot_time = std::clock() - start_time;
  results.erase_time = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  results.erase_count = erase_count;
  results.final_size = test_map.size();
  // std::cout << "  erase time = " << emp::MakeString(result, " seconds.").AsANSIRed() << "\n"
  //           << "  erase_count = " << erase_count << "\n"
  //           << "  final size = " << test_map.size() << std::endl;

  std::cout << "Finished calculating: " << scope << name << std::endl;

  return results;
}

template <typename FUN_T>
void PrintRow(const emp::vector<Results> & result_vec, emp::String trait_name,
              FUN_T fun, const emp::String & color=emp::ANSI::DefaultColor) {
  std::cout << trait_name.AsANSIBold();
  for (Results result : result_vec) {
    emp::String out = emp::MakeString(fun(result));
    size_t width = result.name.size() + 2;
    out.PadBack(' ', width);
    std::cout << color << out << emp::ANSI::DefaultColor;
  }
  std::cout << std::endl;
}

int main(int argc, char * argv[]) {
  auto args = emp::ArgsToStrings(argc, argv);
  bool print_extras = true;
  if (argc > 1) {
    if (args[1] == "--quick" || args[1] == "-q") print_extras = false;
    else {
      std::cerr << "ERROR: Unknown arg '" << args[1] << "'." << std::endl;
      exit(1);
    }
  }

  emp::Random random;
  for (double & val : values) val = random.GetDouble(0.0, 1000000.0);

  size_t cur_key = 0;
  for (size_t & key : keys) {
    cur_key += random.GetUInt(1, MAX_STEP);
    key = cur_key; // *16;
  }
  emp::Shuffle(random, keys);

  Results results;
  emp::vector<Results> result_vec;

  if (print_extras) {
    results = Test<std::unordered_map<size_t, double>>("std::", "unordered_map");
    result_vec.push_back(results);
    // results = Test<std::map<size_t, double>>("std::", "map          ");
    // result_vec.push_back(results);
    // results = Test<std::flat_map<size_t, double>>("std::", "flat_map     ");
    // result_vec.push_back(results);
    results = Test<robin_hood::unordered_map<size_t, double>>("robin_hood::", "unordered_map");
    result_vec.push_back(results);
  }
  results = Test<emp::RobinHoodMap<size_t, double>>("emp::", "RobinHoodMap");
  result_vec.push_back(results);
  results = Test<emp::RobinHoodMap<size_t, double, true>>("emp::", "RobinHoodMap<IMPROVE>");
  result_vec.push_back(results);

  PrintRow(result_vec, "scope         ",
    [](const Results & result){ return result.scope; }, emp::ANSI::Yellow);
  PrintRow(result_vec, "name          ",
    [](const Results & result){ return result.name; }, emp::ANSI::Yellow);
  PrintRow(result_vec, "insert_time.  ",
    [](const Results & result){ return result.insert_time; }, emp::ANSI::BrightGreen);
  PrintRow(result_vec, "search_time   ",
    [](const Results & result){ return result.search_time; }, emp::ANSI::Blue);
  PrintRow(result_vec, "traverse_time ",
    [](const Results & result){ return result.traverse_time; }, emp::ANSI::Green);
  PrintRow(result_vec, "erase_time    ",
    [](const Results & result){ return result.erase_time; }, emp::ANSI::Red);
  PrintRow(result_vec, "found_count   ",
    [](const Results & result){ return result.found_count; }, emp::ANSI::Cyan);
  PrintRow(result_vec, "erase_count   ",
    [](const Results & result){ return result.erase_count; }, emp::ANSI::Cyan);
  PrintRow(result_vec, "start_size    ",
    [](const Results & result){ return result.start_size; }, emp::ANSI::Magenta);
  PrintRow(result_vec, "final_size    ",
    [](const Results & result){ return result.final_size; }, emp::ANSI::Magenta);
  PrintRow(result_vec, "key_total%1M  ",
    [](const Results & result){ return result.key_total % 1000000; }, emp::ANSI::Cyan);
  PrintRow(result_vec, "value_total   ",
    [](const Results & result){ return result.value_total; }, emp::ANSI::Cyan);
}