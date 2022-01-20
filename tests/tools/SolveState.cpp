/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file SolveState.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/SolveState.hpp"

TEST_CASE("Test SolveState", "[tools]")
{
  // Constructor
  emp::SolveState ss0;
  REQUIRE(ss0.GetSize() == 0);

  // Include Exclude Is
  emp::SolveState ss1(10);
  REQUIRE(!ss1.IsIn(0));
  REQUIRE(ss1.IsUnk(0));
  REQUIRE(!ss1.IsFinal());
  ss1.Exclude(0);
  REQUIRE(ss1.IsOut(0));
  ss1.Include(1);
  REQUIRE(ss1.IsIn(1));
  REQUIRE(ss1.IsUnk(2));

  // GetNextUnk
  REQUIRE(ss1.GetNextUnk(0) == 2);

  ss1.Include(2);
  ss1.Exclude(3);
  ss1.Exclude(4);
  ss1.Exclude(5);

  // Count
  REQUIRE(!ss1.IsFinal());
  REQUIRE(ss1.CountIn() == 2);
  REQUIRE(ss1.CountUnk() == 4);
  REQUIRE(ss1.CountOut() == 4);

  // Get Vector
  emp::BitVector vecIn = ss1.GetInVector();
  emp::BitVector vecUnk = ss1.GetUnkVector();
  emp::BitVector vecOut = ss1.GetOutVector();
  REQUIRE( (vecIn | vecUnk | vecOut).All() );
  std::stringstream ss;
  vecIn.PrintBinary(ss);
  REQUIRE(ss.str() == "0000000110");
  ss.str(std::string());
  vecUnk.PrintBinary(ss);
  REQUIRE(ss.str() == "1111000000");
  ss.str(std::string());
  vecOut.PrintBinary(ss);
  REQUIRE(ss.str() == "0000111001");
  ss.str(std::string());

  ss1.Include(6);
  ss1.Exclude(7);
  ss1.Include(8);
  ss1.Include(9);

  // IsFinal ForceExclude
  REQUIRE(ss1.IsFinal());
  ss1.ForceExclude(9);
  REQUIRE(!ss1.IsIn(9));
  REQUIRE(ss1.IsFinal());
  REQUIRE(ss1.IsOut(9));

  // IncludeSet ExcludeSet
  emp::BitVector exc(5);
  exc.SetByte(0,10);
  emp::BitVector inc(5);
  inc.SetByte(0,21);
  emp::SolveState ss2(5);
  ss2.IncludeSet(inc);
  ss2.ExcludeSet(exc);
  REQUIRE(ss2.IsFinal());
  REQUIRE(ss2.CountIn() == 3);
  REQUIRE(ss2.CountOut() == 2);
  REQUIRE(ss2.GetInVector() == inc);
  REQUIRE(ss2.GetOutVector() == exc);
}
