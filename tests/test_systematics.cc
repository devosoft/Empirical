#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#undef NDEBUG
#define TDEBUG 1

#include "third-party/Catch/single_include/catch.hpp"

#include "Evo/Systematics.h"
#include <iostream>

TEST_CASE("Test Systematics", "[evo]")
{
  emp::Systematics<int> sys(true, true, true);

  std::cout << "\nAddOrg 25 (id1, no parent)\n";
  auto id1 = sys.AddOrg(25);
  std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  auto id2 = sys.AddOrg(-10, id1);
  std::cout << "\nAddOrg 26 (id3; parent id1)\n";
  auto id3 = sys.AddOrg(26, id1);
  std::cout << "\nAddOrg 27 (id4; parent id2)\n";
  auto id4 = sys.AddOrg(27, id2);
  std::cout << "\nAddOrg 28 (id5; parent id2)\n";
  auto id5 = sys.AddOrg(28, id2);
  std::cout << "\nAddOrg 29 (id6; parent id5)\n";
  auto id6 = sys.AddOrg(29, id5);
  

  std::cout << "\nRemoveOrg (id2)\n";
  sys.RemoveOrg(id1);
  sys.RemoveOrg(id2);


  double mpd = sys.GetMeanPairwiseDistance();
  std::cout << "MPD: " << mpd <<std::endl;

  REQUIRE(mpd == Approx(2.66666667));

  std::cout << "\nRemoveOrg (id4)\n";
  sys.RemoveOrg(id4);

  std::cout << "id1 = " << id1 << std::endl;
  std::cout << "id2 = " << id2 << std::endl;
  std::cout << "id3 = " << id3 << std::endl;
  std::cout << "id4 = " << id4 << std::endl;

  std::cout << "\nLineage:\n";
  sys.PrintLineage(id4);
  sys.PrintStatus();
}