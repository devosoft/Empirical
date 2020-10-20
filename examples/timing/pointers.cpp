//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some code comparing T*, emp::Ptr<T>, and std::shared_ptr<T>
//
//
//  We are comparing the timings and code complexity for creating a series of N pointers,
//  copying them all, shuffling them, summing their contents, and then deleting them.

#include <algorithm>     // For std::sort
#include <ctime>         // For std::clock
#include <memory>        // For std::shared_ptr
#include <vector>

#include "emp/base/Ptr.hpp"

constexpr size_t N = 1000000;

bool ptr_less(int * p1, int * p2) { return *p1 < *p2; }

int main()
{
  std::clock_t base_start_time = std::clock();

  std::vector<int *> v_base(N);
  for (size_t i = 0; i < N; i++) v_base[i] = new int((int)((i*7)%N));
  std::vector<int *> v_base2(v_base);
  std::sort( v_base2.begin(), v_base2.end(), [](int *p1,int *p2){ return *p1 < *p2; } );
  v_base.resize(0);
  double total_base = 0.0;
  for (int * x : v_base2) total_base += *x;
  for (int * x : v_base2) delete x;
  v_base2.resize(0);

  std::clock_t base_tot_time = std::clock() - base_start_time;
  std::cout << "base sum = " << total_base
            << ";  time = " << 1000.0 * ((double) base_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;


  std::clock_t std_start_time = std::clock();

  std::vector<std::shared_ptr<int>> v_std(N);
  for (size_t i = 0; i < N; i++) v_std[i] = std::make_shared<int>( (int)((i*7)%N) );
  std::vector<std::shared_ptr<int>> v_std2(v_std);
  std::sort( v_std2.begin(), v_std2.end(),
             [](std::shared_ptr<int> p1, std::shared_ptr<int> p2){ return *p1 < *p2; } );
  v_std.resize(0);
  double total_std = 0.0;
  for (std::shared_ptr<int> x : v_std2) total_std += *x;
  v_std2.resize(0);

  std::clock_t std_tot_time = std::clock() - std_start_time;
  std::cout << "std sum = " << total_std
            << ";  time = " << 1000.0 * ((double) std_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;


  std::clock_t emp_start_time = std::clock();

  std::vector<emp::Ptr<int>> v_emp(N);
  for (size_t i = 0; i < N; i++) v_emp[i] = emp::NewPtr<int>((int)((i*7)%N));
  std::vector<emp::Ptr<int>> v_emp2(v_emp);
  std::sort( v_emp2.begin(), v_emp2.end(),
             [](emp::Ptr<int> p1, emp::Ptr<int> p2){ return *p1 < *p2; } );
  v_emp.resize(0);
  double total_emp = 0.0;
  for (emp::Ptr<int> x : v_emp2) total_emp += *x;
  for (emp::Ptr<int> x : v_emp2) x.Delete();
  v_emp2.resize(0);

  std::clock_t emp_tot_time = std::clock() - emp_start_time;
  std::cout << "emp sum = " << total_emp
            << ";  time = " << 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;
}
