/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/Vector.hpp"

TEST_CASE("Test Vector", "[datastructs]")
{
  emp::Vector<int> v1 = {1,2,3,4,5};
  CHECK(v1.IS_STATIC == false);
  CHECK(v1.size() == 5);
  CHECK(v1[3] == 4);

  emp::Vector<int> v2(v1);
  CHECK(v2.IS_STATIC == false);
  CHECK(v2.size() == 5);
  CHECK(v2[1] == 2);

  emp::Vector<char, 20> v3(10, 'a');
  CHECK(v3.IS_STATIC == true);
  CHECK(v3.size() == 10);
  CHECK(v3[7] == 'a');

  emp::Vector<double> v4;
  CHECK(v4.IS_STATIC == false);
  v4.Resize(11, 1.5);
  CHECK(v4.size() == 11);
  CHECK(v4[3] == 1.5);

  double total = 0.0;
  for (double x : v4) total += x;
  CHECK(total == 16.5);

  v4.Fill(10.5, 4, 2);
  total = 0.0;
  for (double x : v4) total += x;
  CHECK(total == 34.5);

  v1.Push(10).Push(11).Push(12);
  CHECK(v1.size() == 8);

  CHECK(v1.Pop() == 12);
  CHECK(v1.Pop() == 11);
  CHECK(v1.size() == 6);
}
