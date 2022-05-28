#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/math.hpp"
#include "emp/data/DataNode.hpp"

#include <sstream>
#include <iostream>
#include <deque>
#include <algorithm>
#include <limits>
#include <numeric>
#include <climits>
#include <unordered_set>
#include <ratio>

TEST_CASE("Test Math", "[math]")
{
  REQUIRE(emp::Mod(5.5, 3.3) == 2.2);
  REQUIRE(emp::MinRef(0,4,-1,6,52) == -1);
  REQUIRE(emp::MaxRef(0,4,-1,6,52) == 52);

  REQUIRE(emp::Log10(100.0) == 2);
  REQUIRE(emp::Ln(emp::E) == 1);
  REQUIRE( emp::Abs(emp::Ln(emp::Exp(5)) - 5) < 0.01);

  REQUIRE(emp::IntLog2(10) == 3);
  REQUIRE(emp::CountOnes(15) == 4);
  REQUIRE(emp::CountOnes(255) == 8);

  unsigned long long large = 0x8000000000000000;
  REQUIRE(emp::MaskHigh<unsigned long long>(1) == large);

  REQUIRE(emp::Min(7,3,100,-50,62) == -50);
  REQUIRE(emp::Max(7,3,100,-50,62) == 100);
}

TEST_CASE("Another Test math", "[math]")
{
  constexpr auto a1 = emp::Log2(3.14);           REQUIRE( a1 > 1.650);   REQUIRE( a1 < 1.651);
  constexpr auto a2 = emp::Log2(0.125);          REQUIRE( a2 == -3.0 );
  constexpr auto a3 = emp::Log(1000, 10);        REQUIRE( a3 == 3.0 );
  constexpr auto a4 = emp::Log(10, 1000);        REQUIRE( a4 > 0.333 );  REQUIRE( a4 < 0.334 );
  constexpr auto a5 = emp::Log10(100);           REQUIRE( a5 == 2.0 );
  constexpr auto a6 = emp::Ln(3.33);             REQUIRE( a6 > 1.202 );  REQUIRE( a6 < 1.204 );
  constexpr auto a7 = emp::Pow2(2.345);          REQUIRE( a7 > 5.080 );  REQUIRE( a7 < 5.081 );
  constexpr auto a8 = emp::Pow(emp::PI, emp::E); REQUIRE( a8 > 22.440 ); REQUIRE( a8 < 22.441 );
  constexpr auto a9 = emp::Pow(7, 10);           REQUIRE( a9 == 282475249 );

  REQUIRE(emp::Mod(10, 7) == 3);
  REQUIRE(emp::Mod(3, 7) == 3);
  REQUIRE(emp::Mod(-4, 7) == 3);
  REQUIRE(emp::Mod(-11, 7) == 3);

  REQUIRE(emp::Mod(-11, 11) == 0);
  REQUIRE(emp::Mod(0, 11) == 0);
  REQUIRE(emp::Mod(11, 11) == 0);

  REQUIRE(emp::Pow(2,3) == 8);
  REQUIRE(emp::Pow(-2,2) == 4);
  REQUIRE(emp::IntPow(3,4) == 81);

  REQUIRE(emp::Min(5) == 5);
  REQUIRE(emp::Min(5,10) == 5);
  REQUIRE(emp::Min(10,5) == 5);
  REQUIRE(emp::Min(40,30,20,10,5,15,25,35) == 5);

  REQUIRE(emp::Max(5) == 5);
  REQUIRE(emp::Max(5,10) == 10);
  REQUIRE(emp::Max(10,5) == 10);
  REQUIRE(emp::Max(40,30,20,10,45,15,25,35) == 45);

  REQUIRE(emp::FloorDivide(0,4) == 0);
  REQUIRE(emp::FloorDivide(1,4) == 0);
  REQUIRE(emp::FloorDivide(2,4) == 0);
  REQUIRE(emp::FloorDivide(3,4) == 0);
  REQUIRE(emp::FloorDivide(4,4) == 1);
  REQUIRE(emp::FloorDivide(6,4) == 1);
  REQUIRE(emp::FloorDivide(5,3) == 1);
  REQUIRE(emp::FloorDivide(6,3) == 2);
  REQUIRE(emp::FloorDivide(7,3) == 2);

  REQUIRE(emp::FloorDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)2,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)3,(size_t)4) == 0);
  REQUIRE(emp::FloorDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::FloorDivide((size_t)6,(size_t)4) == 1);
  REQUIRE(emp::FloorDivide((size_t)5,(size_t)3) == 1);
  REQUIRE(emp::FloorDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::FloorDivide((size_t)7,(size_t)3) == 2);

  REQUIRE(emp::FloorDivide(-1,4) == -1);
  REQUIRE(emp::FloorDivide(-2,4) == -1);
  REQUIRE(emp::FloorDivide(-3,4) == -1);
  REQUIRE(emp::FloorDivide(-4,4) == -1);
  REQUIRE(emp::FloorDivide(-6,4) == -2);
  REQUIRE(emp::FloorDivide(-5,3) == -2);
  REQUIRE(emp::FloorDivide(-6,3) == -2);
  REQUIRE(emp::FloorDivide(-7,3) == -3);

  REQUIRE(emp::FloorDivide(0,-4) == 0);
  REQUIRE(emp::FloorDivide(1,-4) == -1);
  REQUIRE(emp::FloorDivide(2,-4) == -1);
  REQUIRE(emp::FloorDivide(3,-4) == -1);
  REQUIRE(emp::FloorDivide(4,-4) == -1);
  REQUIRE(emp::FloorDivide(6,-4) == -2);
  REQUIRE(emp::FloorDivide(5,-3) == -2);
  REQUIRE(emp::FloorDivide(6,-3) == -2);
  REQUIRE(emp::FloorDivide(7,-3) == -3);

  REQUIRE(emp::FloorDivide(-1,-4) == 0);
  REQUIRE(emp::FloorDivide(-2,-4) == 0);
  REQUIRE(emp::FloorDivide(-3,-4) == 0);
  REQUIRE(emp::FloorDivide(-4,-4) == 1);
  REQUIRE(emp::FloorDivide(-6,-4) == 1);
  REQUIRE(emp::FloorDivide(-5,-3) == 1);
  REQUIRE(emp::FloorDivide(-6,-3) == 2);
  REQUIRE(emp::FloorDivide(-7,-3) == 2);

  REQUIRE(emp::RoundedDivide(0,4) == 0);
  REQUIRE(emp::RoundedDivide(1,4) == 0);
  REQUIRE(emp::RoundedDivide(2,4) == 1);
  REQUIRE(emp::RoundedDivide(3,4) == 1);
  REQUIRE(emp::RoundedDivide(4,4) == 1);
  REQUIRE(emp::RoundedDivide(6,4) == 2);
  REQUIRE(emp::RoundedDivide(5,3) == 2);
  REQUIRE(emp::RoundedDivide(6,3) == 2);
  REQUIRE(emp::RoundedDivide(7,3) == 2);

  REQUIRE(emp::RoundedDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)2,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)3,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)4) == 2);
  REQUIRE(emp::RoundedDivide((size_t)5,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)7,(size_t)3) == 2);

  REQUIRE(emp::RoundedDivide(-1,4) == 0);
  REQUIRE(emp::RoundedDivide(-2,4) == 0);
  REQUIRE(emp::RoundedDivide(-3,4) == -1);
  REQUIRE(emp::RoundedDivide(-4,4) == -1);
  REQUIRE(emp::RoundedDivide(-6,4) == -1);
  REQUIRE(emp::RoundedDivide(-5,3) == -2);
  REQUIRE(emp::RoundedDivide(-6,3) == -2);
  REQUIRE(emp::RoundedDivide(-7,3) == -2);

  REQUIRE(emp::RoundedDivide(0,-4) == 0);
  REQUIRE(emp::RoundedDivide(1,-4) == 0);
  REQUIRE(emp::RoundedDivide(2,-4) == 0);
  REQUIRE(emp::RoundedDivide(3,-4) == -1);
  REQUIRE(emp::RoundedDivide(4,-4) == -1);
  REQUIRE(emp::RoundedDivide(6,-4) == -1);
  REQUIRE(emp::RoundedDivide(5,-3) == -2);
  REQUIRE(emp::RoundedDivide(6,-3) == -2);
  REQUIRE(emp::RoundedDivide(7,-3) == -2);

  REQUIRE(emp::RoundedDivide(-1,-4) == 0);
  REQUIRE(emp::RoundedDivide(-2,-4) == 1);
  REQUIRE(emp::RoundedDivide(-3,-4) == 1);
  REQUIRE(emp::RoundedDivide(-4,-4) == 1);
  REQUIRE(emp::RoundedDivide(-6,-4) == 2);
  REQUIRE(emp::RoundedDivide(-5,-3) == 2);
  REQUIRE(emp::RoundedDivide(-6,-3) == 2);
  REQUIRE(emp::RoundedDivide(-7,-3) == 2);

  REQUIRE(emp::RoundedDivide((size_t)0,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)1,(size_t)4) == 0);
  REQUIRE(emp::RoundedDivide((size_t)2,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)3,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)4,(size_t)4) == 1);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)4) == 2);
  REQUIRE(emp::RoundedDivide((size_t)5,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)6,(size_t)3) == 2);
  REQUIRE(emp::RoundedDivide((size_t)7,(size_t)3) == 2);

  auto MeanUnbiasedDivide = [](int dividend, int divisor, size_t rc){
    emp::Random r = emp::Random(1);
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Log> data;
    for(size_t i=0;i<rc;++i) data.Add(emp::UnbiasedDivide(dividend,divisor,r));
    return data.GetMean();
  };

  REQUIRE(MeanUnbiasedDivide(0,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(1,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(2,4,100) > 0);
  REQUIRE(MeanUnbiasedDivide(2,4,100) < 1);
  REQUIRE(MeanUnbiasedDivide(3,4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(4,4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(6,4,100) > 1);
  REQUIRE(MeanUnbiasedDivide(6,4,100) < 2);
  REQUIRE(MeanUnbiasedDivide(5,3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(6,3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(7,3,100) == 2);

  REQUIRE(MeanUnbiasedDivide(-1,4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(-2,4,100) < 0);
  REQUIRE(MeanUnbiasedDivide(-2,4,100) > -1);
  REQUIRE(MeanUnbiasedDivide(-3,4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(-4,4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(-6,4,100) < -1);
  REQUIRE(MeanUnbiasedDivide(-6,4,100) > -2);
  REQUIRE(MeanUnbiasedDivide(-5,3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(-6,3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(-7,3,100) == -2);

  REQUIRE(MeanUnbiasedDivide(0,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(1,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(2,-4,100) < 0);
  REQUIRE(MeanUnbiasedDivide(2,-4,100) > -1);
  REQUIRE(MeanUnbiasedDivide(3,-4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(4,-4,100) == -1);
  REQUIRE(MeanUnbiasedDivide(6,-4,100) < -1);
  REQUIRE(MeanUnbiasedDivide(6,-4,100) > -2);
  REQUIRE(MeanUnbiasedDivide(5,-3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(6,-3,100) == -2);
  REQUIRE(MeanUnbiasedDivide(7,-3,100) == -2);

  REQUIRE(MeanUnbiasedDivide(-1,-4,100) == 0);
  REQUIRE(MeanUnbiasedDivide(-2,-4,100) > 0);
  REQUIRE(MeanUnbiasedDivide(-2,-4,100) < 1);
  REQUIRE(MeanUnbiasedDivide(-3,-4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(-4,-4,100) == 1);
  REQUIRE(MeanUnbiasedDivide(-6,-4,100) > 1);
  REQUIRE(MeanUnbiasedDivide(-6,-4,100) < 2);
  REQUIRE(MeanUnbiasedDivide(-5,-3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(-6,-3,100) == 2);
  REQUIRE(MeanUnbiasedDivide(-7,-3,100) == 2);

  auto SztMeanUnbiasedDivide = [](size_t dividend, size_t divisor, size_t rc){
    emp::Random r = emp::Random(1);
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Log> data;
    for(size_t i=0;i<rc;++i) data.Add(emp::UnbiasedDivide(dividend,divisor,r));
    return data.GetMean();
  };

  REQUIRE(SztMeanUnbiasedDivide((size_t)0,(size_t)4,100) == 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)1,(size_t)4,100) == 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)2,(size_t)4,100) > 0);
  REQUIRE(SztMeanUnbiasedDivide((size_t)2,(size_t)4,100) < 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)3,(size_t)4,100) == 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)4,(size_t)4,100) == 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)4,100) > 1);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)4,100) < 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)5,(size_t)3,100) == 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)6,(size_t)3,100) == 2);
  REQUIRE(SztMeanUnbiasedDivide((size_t)7,(size_t)3,100) == 2);

  REQUIRE(emp::Sgn(1) == 1);
  REQUIRE(emp::Sgn(2) == 1);
  REQUIRE(emp::Sgn(3) == 1);
  REQUIRE(emp::Sgn(102) == 1);
  REQUIRE(emp::Sgn(0) == 0);
  REQUIRE(emp::Sgn(-1) == -1);
  REQUIRE(emp::Sgn(-2) == -1);
  REQUIRE(emp::Sgn(-3) == -1);
  REQUIRE(emp::Sgn(-102) == -1);

  REQUIRE(emp::Sgn((size_t)1) == 1);
  REQUIRE(emp::Sgn((size_t)2) == 1);
  REQUIRE(emp::Sgn((size_t)3) == 1);
  REQUIRE(emp::Sgn((size_t)102) == 1);
  REQUIRE(emp::Sgn((size_t)0) == 0);

  REQUIRE(emp::Sgn(1.0) == 1);
  REQUIRE(emp::Sgn(2.1) == 1);
  REQUIRE(emp::Sgn(3.0) == 1);
  REQUIRE(emp::Sgn(102.5) == 1);
  REQUIRE(emp::Sgn(0.0) == 0);
  REQUIRE(emp::Sgn(-1.0) == -1);
  REQUIRE(emp::Sgn(-2.1) == -1);
  REQUIRE(emp::Sgn(-3.0) == -1);
  REQUIRE(emp::Sgn(-102.5) == -1);

  REQUIRE(emp::ToRange(-10000, 10, 20) == 10);
  REQUIRE(emp::ToRange(9, 10, 20) == 10);
  REQUIRE(emp::ToRange(10, 10, 20) == 10);
  REQUIRE(emp::ToRange(11, 10, 20) == 11);
  REQUIRE(emp::ToRange(17, 10, 20) == 17);
  REQUIRE(emp::ToRange(20, 10, 20) == 20);
  REQUIRE(emp::ToRange(21, 10, 20) == 20);
  REQUIRE(emp::ToRange(12345678, 10, 20) == 20);
  REQUIRE(emp::ToRange<double>(12345678, 10, 20.1) == 20.1);
  REQUIRE(emp::ToRange(12345678.0, 10.7, 20.1) == 20.1);

  REQUIRE(emp::Factorial(5) == 120);
  REQUIRE(emp::Factorial(3) == 6);

}

TEST_CASE("Test Boolean Math", "[math]")
{
  bool test_bool = true;
  emp::Toggle(test_bool);
  REQUIRE(test_bool == false);

  REQUIRE(emp::AnyTrue(true, false, false, false, true, false) == true);
  REQUIRE(emp::AllTrue(true, false, false, false, true, false) == false);

}

TEST_CASE("Test IsPowerOf2", "[math]") {

  REQUIRE( emp::IsPowerOf2( 0 ) == false );
  REQUIRE( emp::IsPowerOf2( 1 ) );
  REQUIRE( emp::IsPowerOf2( 2 ) );
  REQUIRE( emp::IsPowerOf2( 3 ) == false );
  REQUIRE( emp::IsPowerOf2( 4 ) );
  REQUIRE( emp::IsPowerOf2( 5 ) == false );
  REQUIRE( emp::IsPowerOf2( 6 ) == false );
  REQUIRE( emp::IsPowerOf2( 7 ) == false );
  REQUIRE( emp::IsPowerOf2( 8 ) );
  REQUIRE( emp::IsPowerOf2( 9 ) == false );
  REQUIRE( emp::IsPowerOf2( 10 ) == false );

}

TEST_CASE("Test exp2_overflow_unsafe", "[math]") {

  for (int i = -1022; i <= 1024; ++i) {
    REQUIRE( emp::exp2( i ) == std::exp2( i ) );
  }

}

TEST_CASE("Test exp2f_overflow_unsafe", "[math]") {

  for (int i = -126; i <= 128; ++i) {
    REQUIRE( emp::exp2f( i ) == std::exp2f( i ) );
  }

}

TEST_CASE("Test exp2", "[math]") {

  for (int i = -2000; i <= 2000; ++i) {
    REQUIRE( emp::exp2( i ) == std::exp2( i ) );
  }

}

TEST_CASE("Test exp2f", "[math]") {

  for (int i = -2000; i <= 2000; ++i) {
    REQUIRE( emp::exp2f( i ) == std::exp2f( i ) );
  }

}
