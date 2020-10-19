#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/ContiguousStream.hpp"
#include "emp/math/Random.hpp"

#include <sstream>
#include <iostream>


TEST_CASE("Test ContiguousStreamBuf", "[io]")
{
  emp::Random rand(1);

  std::stringstream ss;
  emp::ContiguousStream cs1;
  emp::ContiguousStream cs2(1);
  emp::ContiguousStream cs3(0);
  emp::ContiguousStream cs4(3);

  std::string temp;

  for (size_t i = 0; i < 3; ++i) {
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Put in some letters
    ss << "Hello_World";
    cs1 << "Hello_World";
    cs2 << "Hello_World";
    cs3 << "Hello_World";
    cs4 << "Hello_World";
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs4.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Make sure flush doesn't break it
    ss << "Will it flush?" << std::flush << "beep";
    cs1 << "Will it flush?" << std::flush << "beep";
    cs2 << "Will it flush?" << std::flush << "beep";
    cs3 << "Will it flush?" << std::flush << "beep";
    cs4 << "Will it flush?" << std::flush << "beep";
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs4.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Put in random data
    for (size_t i = 0; i < 1024; ++i) {
      const auto draw = rand.GetUInt();
      ss << draw;
      cs1 << draw;
      cs2 << draw;
      cs3 << draw;
      cs4 << draw;
    }
    temp = ss.str();

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs1.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs1.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs2.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs2.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs3.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs3.GetSize() );

    REQUIRE((
      std::equal(
        std::begin(temp),
        std::end(temp),
        cs4.cbegin()
      )
    ));
    REQUIRE( temp.size() == cs4.GetSize() );

    // Make sure reset works!
    cs1.Reset();
    cs2.Reset();
    cs3.Reset();
    cs4.Reset();
    ss.str("");
    ss.clear();

  }

}
