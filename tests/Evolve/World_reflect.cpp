/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file World_reflect.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#ifndef NDEBUG
  #define TDEBUG
#endif
#include "emp/Evolve/World.hpp"
#include "emp/Evolve/World_reflect.hpp"

class MyOrg
{
  private:
    double value;
  public:
    explicit MyOrg(double val) : value(val) {}

    double GetFitness(){ return value; }
    double DoMutations(emp::Random& r){ value = value+1.0; return value; }
    double GetValue() { return value; }
    void Print(std::ostream &os) { os << value; }
};

TEST_CASE("Test World reflect", "[Evolve]")
{
  emp::World<double> world;
  world.InjectAt(2.2, 0);
  world.InjectAt(8.5, 1);
  world.InjectAt(9.3, 2);

  std::stringstream ss;
  emp::SetDefaultPrintFun_impl<emp::World<double>, double>(world, true);
  world.Print(ss, "", " ");
  REQUIRE(ss.str() == "2.2 8.5 9.3 ");
  ss.str(std::string());

  #ifndef NDEBUG
  emp::SetDefaultFitFun_impl<emp::World<double>, double>(world);
  double org = 2.2;
  world.CalcFitnessOrg(org);
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  emp::SetDefaultMutFun_impl<emp::World<double>, double>(world);
  world.DoMutationsOrg(org);
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  emp::SetDefaultPrintFun_impl<emp::World<double>, double>(world);
  world.Print();
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();
  #endif

  emp::World<MyOrg> world1;
  world1.InjectAt(MyOrg(7.1), 0);
  world1.InjectAt(MyOrg(2.4), 1);
  world1.InjectAt(MyOrg(11.8), 2);

  MyOrg org3(3.7);
  REQUIRE(world1.CalcFitnessOrg(org3) == 3.7);

  world1.DoMutationsOrg(org3);
  REQUIRE( (org3.GetValue() == 4.7) );

  world1.Print(ss, "", " ");
  REQUIRE(ss.str() == "7.1 2.4 11.8 ");
}
