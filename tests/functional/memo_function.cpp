/**
 * doing any sort of copying of memo_functions seems to give my terminal a
 * stack overflow error (using mingw on windows 10)
 */

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/functional/memo_function.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test memo_function", "[functional]")
{

  /**
   * One arguments
   */

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
  emp::memo_function<uint64_t(int)> empf0;
  empf0 = [](int N){ return N*100000; };
  REQUIRE( empf0(5) == 500000 );

  // move constructor
  uint64_t result = empf0(12);
  emp::memo_function<uint64_t(int)> empf(std::move(empf0));
  REQUIRE( empf(12) == result );

  // Set new std::function
  std::function<uint64_t(int)> stdf = [](int N){ return N/100; };
  empf = stdf;
  REQUIRE( empf(100) == 1 );

  // Move a memo function
  emp::memo_function<uint64_t(int)> empf1;
  empf1 = std::move(empf);
  REQUIRE( empf1(5000) == 50 );

  // Move a std::function
  std::function<char(int)> stdf2 = [](int N){ return (char)(N+96); };
  emp::memo_function<char(int)> empf2;
  empf2 = std::move(stdf2);
  REQUIRE( empf2(1) == 'a' );

  /**
   * Many arguments
   */
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
  // that are not const. Removing the const makes it compile, probably not ideal
  // solution, same with the implicit cast
  // error was: error: passing 'xxxxx' as 'this' argument discards qualifiers
  //std::function<long long(int,int)> m1 = multiply.to_function();
  //std::function<long long(int,int)> m2 = multiply;
  //REQUIRE(m1(5,5) == multiply(5,5));
  //REQUIRE(m2(5,5) == multiply(5,5));

  emp::memo_function<std::string(std::string, std::string)> addWords;
  addWords = [](const std::string & a, const std::string & b){ return a+" "+b; };
  REQUIRE(addWords("Once", "upon") == "Once upon");

  emp::memo_function<std::string(std::string, std::string)> addWords1(std::move(addWords));
  REQUIRE(addWords1("A","house") == "A house");

  //emp::memo_function<std::string(std::string, std::string)> copyAddWords(addWords1);
  //REQUIRE(copyAddWords("Twelve","cars") == "Twelve cars");

  emp::memo_function<std::string(std::string, std::string)> addWords2;
  addWords2 = std::move(addWords1);
  REQUIRE(addWords2("Yellow", "chair") == "Yellow chair");

  std::function<std::string(std::string, std::string)> stdCreateSentence = [](std::string a, const std::string & b){ a[0] = toupper(a[0]); return a+" "+b+"."; };
  emp::memo_function<std::string(std::string, std::string)> createSentence;
  createSentence = std::move(stdCreateSentence);
  REQUIRE(createSentence("he","is") == "He is.");
  // What is Hash function for/how to use?

  /**
   * No arguments
   */
  // memo_function<R>()
  emp::memo_function<double()> returnsFive ([](){ return (double)5.0; });

  // bool Erase Clear Has size
  REQUIRE( (bool) returnsFive );
  REQUIRE( returnsFive.size() == 0 );
  REQUIRE( returnsFive() == 5.0 );
  REQUIRE( returnsFive.size() == 1 );
  REQUIRE( returnsFive.Has() );
  returnsFive.Clear();
  REQUIRE( returnsFive.size() == 0 );
  REQUIRE( !returnsFive.Has() );
  REQUIRE( returnsFive() == 5.0 );
  returnsFive.Erase();
  REQUIRE( returnsFive.size() == 0 );

  // conversions to std::function
  std::function<double()> f1 = returnsFive.to_function();
  std::function<double()> f2 = returnsFive;
  REQUIRE(f1() == returnsFive());
  REQUIRE(f2() == returnsFive());

  // more constructors!
  emp::memo_function<double()> returnsFour;
  returnsFour = [](){ return (double)4.0; };
  REQUIRE(returnsFour() == 4.0);

  emp::memo_function<double()> alsoReturnsFour(std::move(returnsFour));
  REQUIRE(alsoReturnsFour() == 4.0);

  // assignment operators
  emp::memo_function<double()> aMemoFunction;
}

TEST_CASE("Test emp::memo_function", "[functional]")
{
  emp::memo_function<uint64_t(int)> test_fun;

  // Build a Fibonacchi function...
  test_fun = [&test_fun](int N) {
    if (N<=1) return (uint64_t) N;
    return test_fun(N-1) + test_fun(N-2);
  };

  REQUIRE( test_fun(80) == 0x533163ef0321e5 );
}
