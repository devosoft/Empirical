/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/RobinHoodSet.hpp"

TEST_CASE("Test RobinHoodSet", "[datastructs]")
{
  emp::RobinHoodSet<std::string> s;

  CHECK(s.size()  == 0);
  CHECK(s.empty() == true);
  CHECK(s.OK());

  // Basic insert
  auto [it1, ins1] = s.insert("hello");
  CHECK(ins1 == true);
  CHECK(*it1 == "hello");
  CHECK(s.size() == 1);
  CHECK(s.contains("hello"));
  CHECK(!s.empty());

  // Duplicate insert returns false
  auto [it2, ins2] = s.insert("hello");
  CHECK(ins2  == false);
  CHECK(*it2  == "hello");
  CHECK(s.size() == 1);

  // More inserts
  s.insert("world");
  s.insert("foo");
  s.insert("bar");
  CHECK(s.size() == 4);
  CHECK(s.contains("world"));
  CHECK(s.contains("foo"));
  CHECK(s.contains("bar"));
  CHECK(!s.contains("baz"));

  // find
  auto fit = s.find("foo");
  CHECK(fit != s.end());
  CHECK(*fit == "foo");

  auto miss = s.find("missing");
  CHECK(miss == s.end());

  // erase
  CHECK(s.erase("foo") == true);
  CHECK(s.size() == 3);
  CHECK(!s.contains("foo"));
  CHECK(s.erase("foo") == false);  // already gone

  // iteration visits every key exactly once
  emp::vector<std::string> seen;
  for (const std::string & k : s) seen.push_back(k);
  CHECK(seen.size() == 3);
  // All remaining keys present
  CHECK(std::find(seen.begin(), seen.end(), "hello") != seen.end());
  CHECK(std::find(seen.begin(), seen.end(), "world") != seen.end());
  CHECK(std::find(seen.begin(), seen.end(), "bar")   != seen.end());

  // clear
  s.clear();
  CHECK(s.size()  == 0);
  CHECK(s.empty() == true);
  CHECK(s.OK());

  // Initializer-list constructor
  emp::RobinHoodSet<int> ns{1, 2, 3, 4, 5};
  CHECK(ns.size() == 5);
  CHECK(ns.contains(3));
  CHECK(!ns.contains(6));

  // reserve then bulk-insert
  emp::RobinHoodSet<int> big;
  big.reserve(200);
  for (int i = 0; i < 200; ++i) big.insert(i);
  CHECK(big.size() == 200);
  CHECK(big.OK());
  for (int i = 0; i < 200; ++i) CHECK(big.contains(i));

  // Analysis helpers compile and run
  [[maybe_unused]] auto ave_offset = big.CalcAveOffset();

  // copy / move
  emp::RobinHoodSet<int> copy = big;
  CHECK(copy.size() == 200);
  CHECK(copy.contains(99));

  emp::RobinHoodSet<int> moved = std::move(copy);
  CHECK(moved.size() == 200);
  CHECK(moved.contains(99));

  // IMPROVE_HASH variant
  emp::RobinHoodSet<std::string, true> ihs;
  ihs.insert("a"); ihs.insert("b"); ihs.insert("c");
  CHECK(ihs.size() == 3);
  CHECK(ihs.contains("a"));
  CHECK(ihs.OK());
}

// Local settings for Empecable file checker.
// empecable_words: ns ins ins1 ins2 it1 it2 ihs
