/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file IndexMap.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/IndexMap.hpp"

TEST_CASE("Test IndexMap", "[datastructs]")
{
  emp::IndexMap im(4, 2.5);
  REQUIRE(im.GetSize() == 4);

  REQUIRE(im.GetWeight(0) == 2.5);
  im.Adjust(0, 3.0);
  REQUIRE(im.GetWeight(0) == 3.0);

  im.ResizeClear(3);
  REQUIRE(im.size() == 3);
  REQUIRE(im.GetWeight(0) == 0);
  im[0] = 1;
  im[1] = 2;
  REQUIRE(im.GetWeight(0) == 1);
  REQUIRE(im.GetWeight(1) == 2);
  REQUIRE(im.RawWeight(2) == 1);
  REQUIRE(im.RawWeight(3) == 2);

  im.resize(5);
  REQUIRE(im.size() == 5);
  REQUIRE(im.GetWeight(0) == 1);
  REQUIRE(im.RawWeight(7) == 1);

  im[2] = 3;
  im[3] = 4;
  im[4] = 5;

  REQUIRE(im.RawProb(7) == (1.0/15.0));
  REQUIRE(im.GetProb(4) == (1.0/3.0));

  emp::IndexMap im2(5);
  emp::vector<double> new_weights;
  new_weights.push_back(0);
  new_weights.push_back(5);
  new_weights.push_back(10);
  new_weights.push_back(15);
  new_weights.push_back(20);

  im2.Adjust(new_weights);
  REQUIRE(im2[0] == 0);
  REQUIRE(im2[1] == 5);
  REQUIRE(im2[2] == 10);
  REQUIRE(im2[3] == 15);
  REQUIRE(im2[4] == 20);

  im2 += im;
  REQUIRE(im2[0] == 1);
  REQUIRE(im2[1] == 7);
  REQUIRE(im2[2] == 13);
  REQUIRE(im2[3] == 19);
  REQUIRE(im2[4] == 25);

  im2 -= im;

  REQUIRE(im2[0] == 0);
  REQUIRE(im2[1] == 5);
  REQUIRE(im2[2] == 10);
  REQUIRE(im2[3] == 15);
  REQUIRE(im2[4] == 20);
}

// TODO: add moar asserts
TEST_CASE("Another Test IndexMap", "[datastructs]")
{
  emp::IndexMap imap(8);
  imap[0] = 1.0;
  imap[1] = 1.0;
  imap[2] = 1.0;
  imap[3] = 1.0;
  imap[4] = 2.0;
  imap[5] = 2.0;
  imap[6] = 0.0;
  imap[7] = 8.0;

  REQUIRE(imap.GetSize() == 8);
  REQUIRE(imap.GetWeight() == 16.0);
  REQUIRE(imap.GetWeight(2) == 1.0);
  REQUIRE(imap.GetWeight(5) == 2.0);
  REQUIRE(imap.GetWeight(7) == 8.0);
  REQUIRE(imap[5] == 2.0);
  REQUIRE(imap.GetProb(4) == 0.125);
  REQUIRE(imap.GetProb(7) == 0.5);
  REQUIRE(imap.Index(7.1) == 5);

  // Add a new element to the end of the map that takes up half of the weight.
  imap.PushBack(16.0);

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 32.0);
  REQUIRE(imap.GetWeight(2) == 1.0);
  REQUIRE(imap.GetWeight(5) == 2.0);
  REQUIRE(imap.GetWeight(7) == 8.0);
  REQUIRE(imap.GetWeight(8) == 16.0);
  REQUIRE(imap[5] == 2.0);
  REQUIRE(imap.GetProb(7) == 0.25);
  REQUIRE(imap.Index(7.1) == 5);
  REQUIRE(imap.Index(17.1) == 8);

  emp::IndexMap imap_bak(imap);
  imap.AdjustAll(10.0);

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 90.0);
  REQUIRE(imap.GetWeight(2) == 10.0);
  REQUIRE(imap.GetWeight(8) == 10.0);
  REQUIRE(imap[5] == 10.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(75.0) == 7);

  // Did the backup copy work correctly?
  REQUIRE(imap_bak.GetSize() == 9);
  REQUIRE(imap_bak.GetWeight() == 32.0);
  REQUIRE(imap_bak.GetWeight(2) == 1.0);
  REQUIRE(imap_bak.GetWeight(5) == 2.0);
  REQUIRE(imap_bak.GetWeight(7) == 8.0);
  REQUIRE(imap_bak.GetWeight(8) == 16.0);
  REQUIRE(imap_bak[5] == 2.0);
  REQUIRE(imap_bak.GetProb(7) == 0.25);
  REQUIRE(imap_bak.Index(7.1) == 5);
  REQUIRE(imap_bak.Index(17.1) == 8);

  // Can we add on values from one index map to another?
  imap += imap_bak;

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 122.0);
  REQUIRE(imap.GetWeight(2) == 11.0);
  REQUIRE(imap.GetWeight(5) == 12.0);
  REQUIRE(imap.GetWeight(7) == 18.0);
  REQUIRE(imap.GetWeight(8) == 26.0);
  REQUIRE(imap[5] == 12.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(90.0) == 7);

  // And subtraction?
  imap -= imap_bak;

  REQUIRE(imap.GetSize() == 9);
  REQUIRE(imap.GetWeight() == 90.0);
  REQUIRE(imap.GetWeight(2) == 10.0);
  REQUIRE(imap.GetWeight(8) == 10.0);
  REQUIRE(imap[5] == 10.0);
  REQUIRE(imap.Index(7.1) == 0);
  REQUIRE(imap.Index(75.0) == 7);
}
