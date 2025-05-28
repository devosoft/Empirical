#include <ctime>
#include <iostream>
#include <map>
#include <unordered_map>

#include "../../include/emp/base/vector.hpp"
#include "../../include/emp/datastructs/RobinHoodMap.hpp"
#include "../../include/emp/math/Random.hpp"
#include "../../include/emp/tools/String.hpp"
#include "../../include/emp/tools/string_utils.hpp"

int main() {
  static constexpr size_t COUNT = 10000000;

  emp::Random random;
  emp::vector<double> values(COUNT);
  for (double & val : values) val = random.GetDouble(0.0, 1000000.0);




  emp::RobinHoodMap<size_t, double> test_rh_map;

  // Test inserts
  std::clock_t start_time = std::clock();
  for (size_t i = 0; i < values.size(); ++i) {
    test_rh_map.Insert(i, values[i]);
  }
  std::clock_t tot_time = std::clock() - start_time;
  double result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Robin Hood Map with " << COUNT << " insertions\n"
            << "  insert time = " << emp::MakeString(result, " seconds.").AsANSIBlue() << "\n"
            << "  size = "     << test_rh_map.size() << std::endl
            << "  capacity = " << test_rh_map.capacity() << std::endl;

  // Test searching
  start_time = std::clock();
  size_t found_count = 0;
  for (size_t i = 0; i < values.size() * 2; ++i) {
    if (test_rh_map.FindPtr(i) != nullptr) ++found_count;
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "  search time = " << emp::MakeString(result, " seconds.").AsANSIGreen() << "\n"
            << "  found_count = " << found_count << "\n";

  // Test deletion
  start_time = std::clock();
  size_t erase_count = 0;
  for (size_t i = values.size(); i >= 500; --i) {
    if (test_rh_map.erase(i)) ++erase_count;
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "  erase time = " << emp::MakeString(result, " seconds.").AsANSIRed() << "\n"
            << "  erase_count = " << erase_count << "\n"
            << "  final size = " << test_rh_map.size() << std::endl;

  std::cout << std::endl;






  std::unordered_map<size_t, double> test_uo_map;

  start_time = std::clock();

  for (size_t i = 0; i < values.size(); ++i) {
    test_uo_map.insert({i, values[i]});
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Unordered Map with " << COUNT << " insertions\n"
            << "  time = " << emp::MakeString(result, " seconds.").AsANSIBlue() << "\n"
            << "  size = "     << test_uo_map.size() << std::endl;

  // Test searching
  start_time = std::clock();
  found_count = 0;
  for (size_t i = 0; i < values.size() * 2; ++i) {
    if (test_uo_map.contains(i)) ++found_count;
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "  search time = " << emp::MakeString(result, " seconds.").AsANSIGreen() << "\n"
            << "  found_count = " << found_count << "\n";

  // Test deletion
  start_time = std::clock();
  erase_count = 0;
  for (size_t i = values.size(); i >= 500; --i) {
    if (test_uo_map.erase(i)) ++erase_count;
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "  erase time = " << emp::MakeString(result, " seconds.").AsANSIRed() << "\n"
            << "  erase_count = " << erase_count << "\n"
            << "  final size = " << test_uo_map.size() << std::endl;

  std::cout << std::endl;







  std::map<size_t, double> test_sorted_map;

  start_time = std::clock();

  for (size_t i = 0; i < values.size(); ++i) {
    test_sorted_map.insert({i, values[i]});
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Ordered Map with " << COUNT << " insertions\n"
            << "  time = " << emp::MakeString(result, " seconds.").AsANSIBlue() << "\n"
            << "  size = "     << test_sorted_map.size() << std::endl;

  // Test searching
  start_time = std::clock();
  found_count = 0;
  for (size_t i = 0; i < values.size() * 2; ++i) {
    if (test_sorted_map.contains(i)) ++found_count;
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "  search time = " << emp::MakeString(result, " seconds.").AsANSIGreen() << "\n"
            << "  found_count = " << found_count << "\n";

  // Test deletion
  start_time = std::clock();
  erase_count = 0;
  for (size_t i = values.size(); i >= 500; --i) {
    if (test_sorted_map.erase(i)) ++erase_count;
  }
  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "  erase time = " << emp::MakeString(result, " seconds.").AsANSIRed() << "\n"
            << "  erase_count = " << erase_count << "\n"
            << "  final size = " << test_sorted_map.size() << std::endl;

  std::cout << std::endl;

}