#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include "emp/base/array.hpp"
#include "emp/Evolve/NK-const.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/Evolve/Resource.hpp"
#include "emp/Evolve/NK.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

constexpr size_t K = 3;
constexpr size_t N = 40;
using BitOrg = emp::BitSet<N>;


TEST_CASE("Test resources", "[evo]")
{
  size_t POP_SIZE = 100;

  emp::Random random(1);
  emp::World<BitOrg> pop(random);
  pop.SetPopStruct_Mixed(true);
  pop.SetMutFun([](BitOrg & org, emp::Random & r){return 0;});

  emp::vector<emp::Resource> resources;
  resources.push_back(emp::Resource(100, 100, .01));
  resources.push_back(emp::Resource(100, 100, .01));
  resources.push_back(emp::Resource(100, 100, .01));

  pop.OnUpdate([&resources](int ud){
      for (emp::Resource& res : resources) {
          res.Update();
      }
  });

  REQUIRE(resources[0].GetAmount() == 100);

  pop.Update();

  REQUIRE(resources[0].GetAmount() == Approx(199.0));

  pop.Update();
  pop.Update();
  pop.Update();
  pop.Update();
  pop.Update();

  // Build a random initial population
  for (size_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (size_t j = 0; j < N; j++) next_org[j] = 0;
    pop.Inject(next_org);
  }

  pop.SetFitFun([](BitOrg &org){ return 10; });


  emp::vector<std::function<double(BitOrg&)> > fit_funs;

  fit_funs.push_back([](BitOrg &org){ return org.CountOnes()/N; });
  fit_funs.push_back([](BitOrg &org){ return org[0]; });
  fit_funs.push_back([](BitOrg &org){ return 1 - org[0]; });

  emp::ResourceSelect(pop, fit_funs, resources, 5, POP_SIZE);

  REQUIRE(resources[0].GetAmount() == Approx(779.346));
  REQUIRE(resources[1].GetAmount() == Approx(779.346));
  REQUIRE(resources[2].GetAmount() == Approx(617.265));

}
