/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/geometry/Box2D.hpp"

TEST_CASE("Test Box2D", "[geometry]")
{
  emp::Point UL{12.5, 45.75};
  emp::Size2D size{10, 10};
  emp::Box2D box{UL, size};

  CHECK( box.GetUL() == UL );
  CHECK( box.GetSize() == size );
  CHECK( box.GetLeft() == 12.5 );
  CHECK( box.GetRight() == 22.5 );
  CHECK( box.GetTop() == 45.75 );
  CHECK( box.GetBottom() == 55.75 );
  CHECK( box.GetArea() == 100 );
  CHECK( box.GetPerimeter() == 40 );

  box.SetPosition({-5, -5});

  CHECK( box.GetUL() == emp::Point{-5, -5} );
  CHECK( box.GetSize() == size );
  CHECK( box.GetLeft() == -5 );
  CHECK( box.GetRight() == 5 );
  CHECK( box.GetTop() == -5 );
  CHECK( box.GetBottom() == 5 );
  CHECK( box.GetArea() == 100 );
  CHECK( box.GetPerimeter() == 40 );

  box.SetSize({20,20});

  CHECK( box.GetUL() == emp::Point{-5, -5} );
  CHECK( box.GetSize() == size * 2 );
  CHECK( box.GetLeft() == -5 );
  CHECK( box.GetRight() == 15 );
  CHECK( box.GetTop() == -5 );
  CHECK( box.GetBottom() == 15 );
  CHECK( box.GetArea() == 400 );
  CHECK( box.GetPerimeter() == 80 );

  box += emp::Point{10,10};

  CHECK( box.GetUL() == emp::Point{5, 5} );
  CHECK( box.GetSize() == size * 2 );
  CHECK( box.GetLeft() == 5 );
  CHECK( box.GetRight() == 25 );
  CHECK( box.GetTop() == 5 );
  CHECK( box.GetBottom() == 25 );
  CHECK( box.GetArea() == 400 );
  CHECK( box.GetPerimeter() == 80 );

  box *= 1.5;

  CHECK( box.GetUL() == emp::Point{5, 5} );
  CHECK( box.GetSize() == size * 3 );
  CHECK( box.GetLeft() == 5 );
  CHECK( box.GetRight() == 35 );
  CHECK( box.GetTop() == 5 );
  CHECK( box.GetBottom() == 35 );
  CHECK( box.GetArea() == 900 );
  CHECK( box.GetPerimeter() == 120 );
}
