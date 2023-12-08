/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/DataMap.hpp"

TEST_CASE("Test DataMap", "[data]")
{
  emp::DataMap dm;
  dm.AddVar<double>("PI", 3.14, "The constant value PI.");
  dm.AddVar<std::string>("name", "emp", "The name of this library.");

  size_t id_pi = dm.GetID("PI");
  size_t id_name = dm.GetID("name");

  emp::DataMap dm2(dm);
  dm2.Set("PI", 3.1415);

  CHECK( dm.Get<double>("PI") == 3.14 );
  CHECK( dm.Get<double>(id_pi) == 3.14 );
  CHECK( dm2.Get<double>("PI") == 3.1415 );
  CHECK( dm2.GetAsDouble(id_pi) == 3.1415 );

  CHECK( dm.Get<std::string>("name") == "emp" );
  CHECK( dm2.Get<std::string>("name") == "emp" );
  dm2.Set<std::string>( "name", "Empirical" );
  CHECK( dm.Get<std::string>("name") == "emp" );
  CHECK( dm2.Get<std::string>("name") == "Empirical" );

  dm = dm2;
  CHECK( dm.Get<double>("PI") == 3.1415 );
  CHECK( dm2.Get<double>("PI") == 3.1415 );
  CHECK( dm.Get<std::string>(id_name) == "Empirical" );
  CHECK( dm2.Get<std::string>(id_name) == "Empirical" );
  CHECK( dm.SameLayout(dm2) == true );

  dm.AddVar<char>("letter", 'q');
  CHECK( dm.HasName("letter") == true );
  CHECK( dm2.HasName("letter") == false );

  CHECK( dm.IsType<char>("letter") == true );
  CHECK( dm.IsType<int>("letter") == false );

  dm.Get<char>("letter") = 'z';
  size_t id_letter = dm.GetID("letter");
  CHECK( dm.GetAsString(id_letter) == "z" );

  CHECK( dm.SameLayout(dm2) == false );

  CHECK( dm.IsLocked() == false );
  dm.LockLayout();
  CHECK( dm.IsLocked() == true );
  CHECK( dm2.IsLocked() == false );


  // Test putting multiple entries in a DataMap at once.
  dm2.AddVar<double>("array", 0.0, "Test Array of doubles", "some notes.", 20);
  std::span<double> test_span = dm2.Get<double>("array", 20);
  test_span[10] = 3.14;
  test_span[17] = 3.14;
  std::span<double> test_span2 = dm2.Get<double>("array", 20);

  double total = 0.0;
  for (double x : test_span2) { total += x; }
  CHECK( total == 3.14 + 3.14);
}
