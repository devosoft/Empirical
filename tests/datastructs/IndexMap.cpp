/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-2022
*/
/**
 *  @file
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/IndexMap.hpp"

TEST_CASE("Test IndexMap", "[datastructs]")
{
  emp::IndexMap im(4, 2.5);
  CHECK(im.GetSize() == 4);

  CHECK(im.GetWeight(0) == 2.5);
  im.Set(0, 3.0);
  CHECK(im.GetWeight(0) == 3.0);

  im.ResizeClear(3);
  CHECK(im.size() == 3);
  CHECK(im.GetWeight(0) == 0);
  im[0] = 1;
  im[1] = 2;
  CHECK(im.GetWeight(0) == 1);
  CHECK(im.GetWeight(1) == 2);

  im.resize(5);
  CHECK(im.size() == 5);
  CHECK(im.GetWeight(0) == 1);

  im[2] = 3;
  im[3] = 4;
  im[4] = 5;

  CHECK(im.GetProb(4) == (1.0/3.0));

  emp::IndexMap im2(5);
  emp::vector<double> new_weights;
  new_weights.push_back(0);
  new_weights.push_back(5);
  new_weights.push_back(10);
  new_weights.push_back(15);
  new_weights.push_back(20);

  im2.Set(new_weights);
  CHECK(im2[0] == 0);
  CHECK(im2[1] == 5);
  CHECK(im2[2] == 10);
  CHECK(im2[3] == 15);
  CHECK(im2[4] == 20);

  im2 += im;
  CHECK(im2[0] == 1);
  CHECK(im2[1] == 7);
  CHECK(im2[2] == 13);
  CHECK(im2[3] == 19);
  CHECK(im2[4] == 25);

  im2 -= im;

  CHECK(im2[0] == 0);
  CHECK(im2[1] == 5);
  CHECK(im2[2] == 10);
  CHECK(im2[3] == 15);
  CHECK(im2[4] == 20);
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

  CHECK(imap.GetSize() == 8);
  CHECK(imap.GetWeight() == 16.0);
  CHECK(imap.GetWeight(2) == 1.0);
  CHECK(imap.GetWeight(5) == 2.0);
  CHECK(imap.GetWeight(7) == 8.0);
  CHECK(imap[5] == 2.0);
  CHECK(imap.GetProb(4) == 0.125);
  CHECK(imap.GetProb(7) == 0.5);
  CHECK(imap.Index(7.1) == 5);

  // Add a new element to the end of the map that takes up half of the weight.
  imap.PushBack(16.0);

  CHECK(imap.GetSize() == 9);
  CHECK(imap.GetWeight() == 32.0);
  CHECK(imap.GetWeight(2) == 1.0);
  CHECK(imap.GetWeight(5) == 2.0);
  CHECK(imap.GetWeight(7) == 8.0);
  CHECK(imap.GetWeight(8) == 16.0);
  CHECK(imap[5] == 2.0);
  CHECK(imap.GetProb(7) == 0.25);
  CHECK(imap.Index(7.1) == 5);
  CHECK(imap.Index(17.1) == 8);

  emp::IndexMap imap_bak(imap);
  imap.SetAll(10.0);

  CHECK(imap.GetSize() == 9);
  CHECK(imap.GetWeight() == 90.0);
  CHECK(imap.GetWeight(2) == 10.0);
  CHECK(imap.GetWeight(8) == 10.0);
  CHECK(imap[5] == 10.0);
  CHECK(imap.Index(7.1) == 0);
  CHECK(imap.Index(75.0) == 7);

  // Did the backup copy work correctly?
  CHECK(imap_bak.GetSize() == 9);
  CHECK(imap_bak.GetWeight() == 32.0);
  CHECK(imap_bak.GetWeight(2) == 1.0);
  CHECK(imap_bak.GetWeight(5) == 2.0);
  CHECK(imap_bak.GetWeight(7) == 8.0);
  CHECK(imap_bak.GetWeight(8) == 16.0);
  CHECK(imap_bak[5] == 2.0);
  CHECK(imap_bak.GetProb(7) == 0.25);
  CHECK(imap_bak.Index(7.1) == 5);
  CHECK(imap_bak.Index(17.1) == 8);

  // Can we add on values from one index map to another?
  imap += imap_bak;

  CHECK(imap.GetSize() == 9);
  CHECK(imap.GetWeight() == 122.0);
  CHECK(imap.GetWeight(2) == 11.0);
  CHECK(imap.GetWeight(5) == 12.0);
  CHECK(imap.GetWeight(7) == 18.0);
  CHECK(imap.GetWeight(8) == 26.0);
  CHECK(imap[5] == 12.0);
  CHECK(imap.Index(7.1) == 0);
  CHECK(imap.Index(90.0) == 7);

  // And subtraction?
  imap -= imap_bak;

  CHECK(imap.GetSize() == 9);
  CHECK(imap.GetWeight() == 90.0);
  CHECK(imap.GetWeight(2) == 10.0);
  CHECK(imap.GetWeight(8) == 10.0);
  CHECK(imap[5] == 10.0);
  CHECK(imap.Index(7.1) == 0);
  CHECK(imap.Index(75.0) == 7);
}
