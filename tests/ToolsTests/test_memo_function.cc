#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/memo_function.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test memo_function", "[tools]")
{
  // memo_function<R(ARG)>()
  emp::memo_function<uint64_t(int)> factorial(
  [&factorial](int N) {
    if (N<=1) return (uint64_t)1;
    return factorial(N-1)*N;
  });
  
  // bool size Has Clear Erase
  REQUIRE( (bool)factorial );
  REQUIRE( factorial(5) == 120 );
  REQUIRE( factorial(15) == (uint64_t)1307674368000 );
  REQUIRE( factorial.size() == 15 );
  REQUIRE( factorial(10) == (uint64_t)3628800 );
  REQUIRE( factorial.size() == 15 );
  REQUIRE( factorial.Has(7) );
  factorial.Clear();
  REQUIRE( factorial.size() == 0 );
  REQUIRE( !factorial.Has(5) );
  factorial(15);
  REQUIRE( factorial.Has(5) );
  factorial.Erase(5);
  REQUIRE( !factorial.Has(5) );
  
  // conversions to std::function<R(ARG)> type
  std::function<uint64_t(int)> fx = factorial.to_function();
  std::function<uint64_t(int)> fn = factorial;
  REQUIRE(fx(5) == factorial(5));
  REQUIRE(fn(5) == factorial(5));
  
  // another constructor
  emp::memo_function<uint64_t(int)> different;
  different = [](int N){ return N*100000; };
  REQUIRE( different(5) == 500000 );
  
  // memo_function<R(A1,A2,EXTRA...)>()
  emp::memo_function<long long(int,int)> multiply(
    [](int a, int b){ return a*b; });
  
  // size Has Clear Erase bool
  REQUIRE( (bool) multiply );
  REQUIRE( multiply.size() == 0 );
  REQUIRE( multiply(5,5) == (long long)25 );
  REQUIRE( multiply.size() == 1 );
  REQUIRE( multiply.Has(5,5) );
  multiply.Clear();
  REQUIRE( !multiply.Has(5,5) );
  REQUIRE( multiply.size() == 0 );
  REQUIRE( multiply(2,3) == (long long)6 );
  REQUIRE( multiply.Has(2,3) );
  multiply.Erase(2,3);
  REQUIRE( !multiply.Has(2,3) );
  
  // conversions to std::function type
  // COMMENT: to_function is const, but has issues with calling other functions
  // that are not const. Removing the const makes it compile. Adding const to
  // other functions is probably more ideal, but it was more than just one so
  // I strayed away from that idea
  // same with the implicit cast 
  // error was: error: passing 'xxxxx' as 'this' argument discards qualifiers
  std::function<long long(int,int)> m1 = multiply.to_function();
  std::function<long long(int,int)> m2 = multiply;
  REQUIRE(m1(5,5) == multiply(5,5));
  REQUIRE(m2(5,5) == multiply(5,5));
  
  // What is Hash function for/how to use?
  
  // memo_function<R>()
  emp::memo_function<double()> returnsfive ([](){ return (double)5.0; });
  
  // bool Erase Clear Has size
  REQUIRE( (bool) returnsfive );
  REQUIRE( returnsfive.size() == 0 );
  REQUIRE( returnsfive() == 5.0 );
  REQUIRE( returnsfive.size() == 1 );
  REQUIRE( returnsfive.Has() );
  returnsfive.Clear();
  REQUIRE( returnsfive.size() == 0 );
  REQUIRE( !returnsfive.Has() );
  REQUIRE( returnsfive() == 5.0 );
  returnsfive.Erase();
  REQUIRE( returnsfive.size() == 0 );
  
  // conversions to std::function
  std::function<double()> f1 = returnsfive.to_function();
  std::function<double()> f2 = returnsfive;
  REQUIRE(f1() == returnsfive());
  REQUIRE(f2() == returnsfive());
  
  // more constructors!
  emp::memo_function<double()> returnsfour;
  returnsfour = [](){ return (double)4.0; };
  REQUIRE(returnsfour() == 4.0);
}