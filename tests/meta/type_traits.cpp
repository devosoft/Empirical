/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file type_traits.cpp
 */

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/meta/type_traits.hpp"

struct ExampleType1 {
  std::string ToString() { return "example"; }
  void FromString(const std::string & in) { (void) in; }
  // double ToDouble() { return 42.0; }
};
struct ExampleType2 {
  // std::string ToString() { return "example"; }
  double ToDouble() { return 42.0; }
  void FromDouble(double in) { (void) in; }
};

TEST_CASE("Test type_traits", "[meta]")
{
  REQUIRE( emp::IsIterable<int>() == false );
  REQUIRE( emp::IsIterable<std::vector<int>>() == true );
  REQUIRE( emp::IsIterable<std::string>() == true);

  REQUIRE( emp::hasToString<ExampleType1> == true );
  REQUIRE( emp::hasToString<ExampleType2> == false );
  REQUIRE( emp::hasToString<std::string> == false );
  REQUIRE( emp::hasToString<int> == false );

  REQUIRE( emp::hasToDouble<ExampleType1> == false );
  REQUIRE( emp::hasToDouble<ExampleType2> == true );
  REQUIRE( emp::hasToDouble<std::string> == false );
  REQUIRE( emp::hasToDouble<int> == false );

  REQUIRE( emp::hasFromString<ExampleType1> == true );
  REQUIRE( emp::hasFromString<ExampleType2> == false );
  REQUIRE( emp::hasFromString<std::string> == false );
  REQUIRE( emp::hasFromString<int> == false );

  REQUIRE( emp::hasFromDouble<ExampleType1> == false );
  REQUIRE( emp::hasFromDouble<ExampleType2> == true );
  REQUIRE( emp::hasFromDouble<std::string> == false );
  REQUIRE( emp::hasFromDouble<int> == false );

  REQUIRE( emp::is_std_function<int>() == false );
  REQUIRE( emp::is_std_function<int(double)>() == false );
  REQUIRE( emp::is_std_function<std::function<int(double)>>() == true );

  REQUIRE( std::is_same< emp::remove_std_function_t<std::function<int(double)>>, int(double) >() == true );

  REQUIRE( std::is_same< emp::element_t<int>, int >() == true );
  REQUIRE( std::is_same< emp::element_t<std::vector<int>>, int >() == true );

  REQUIRE( emp::is_emp_vector<char>() == false );
  REQUIRE( emp::is_emp_vector<emp::vector<std::string>>() == true );

  REQUIRE( emp::is_ptr_type< bool >() == false);
  REQUIRE( emp::is_ptr_type< bool * >() == true);
  REQUIRE( emp::is_ptr_type< bool * const >() == true);
  REQUIRE( emp::is_ptr_type< emp::Ptr<bool> >() == true);
  REQUIRE( emp::is_ptr_type< bool & >() == false);

  REQUIRE( std::is_same< emp::remove_pointer_t< bool >, bool >() == true );
  REQUIRE( std::is_same< emp::remove_pointer_t< bool * >, bool >() == true );
  REQUIRE( std::is_same< emp::remove_pointer_t< bool * const >, const bool >() == true );
  REQUIRE( std::is_same< emp::remove_pointer_t< emp::Ptr<bool> >, bool >() == true );
  REQUIRE( std::is_same< emp::remove_pointer_t< bool & >, bool & >() == true );
}

TEST_CASE("Test is_streamable", "[meta]") {

  REQUIRE( emp::is_streamable<int>::value == true );
  REQUIRE( emp::is_streamable<std::string>::value == true );
  REQUIRE( emp::is_streamable<std::function<void()>>::value == false );

}
