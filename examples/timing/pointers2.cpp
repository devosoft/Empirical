#include <algorithm>     // For std::sort
#include <chrono>        // For steady_clock
#include <concepts>      // For std::same_as
#include <memory>        // For std::shared_ptr
#include <print>
#include <vector>

#include "emp/base/Ptr.hpp"

constexpr size_t N = 10000000;

template <typename PTR_T, typename MAKE_FUN_T>
void TestPointers(const std::string & name, MAKE_FUN_T make_fun) {
  using clock = std::chrono::steady_clock;
  auto start = clock::now();

  // Build a vector of pointers to test.
  std::vector<PTR_T> ptr_vec(N);  
  for (size_t i = 0; i < N; i++) ptr_vec[i] = make_fun((i*7)%N);

  // Try sorting the values.
  std::sort( ptr_vec.begin(), ptr_vec.end(),
    [](PTR_T & p1, PTR_T & p2){ return *p1 < *p2; } );

  // Try reading and using all of the values.
  int64_t total = 0;
  for (PTR_T & x : ptr_vec) total += *x;
  if constexpr (std::same_as<PTR_T, int*>) {
    for (PTR_T x : ptr_vec) delete x;
  }
  ptr_vec.resize(0);

  auto ms = std::chrono::duration<double, std::milli>(clock::now() - start).count();
  std::println("{:>8} time = {:>8.3f} ms; total = {}", name, ms, total);
}

int main()
{
  TestPointers<int*>("Raw", [](int X){ return new int{X}; });
  TestPointers<std::shared_ptr<int>>("Shared", [](int X){ return std::make_shared<int>(X); });
  TestPointers<std::unique_ptr<int>>("Unique", [](int X){ return std::make_unique<int>(X); });
  TestPointers<int*>("Raw", [](int X){ return new int{X}; });
}
