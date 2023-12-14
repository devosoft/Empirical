/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-2022
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/StreamManager.hpp"

TEST_CASE("Test StreamManager", "[io]")
{
  emp::StreamManager sm;

  std::stringstream & test_stream = sm.AddStringStream("test_stream");
  test_stream << "abc" << 123 << 'd' << 'e' << 'f';
  REQUIRE( test_stream.str() == "abc123def" );

  std::iostream & test_stream2 = sm.AddIOStream("test_stream2");
  test_stream2 << "This is the second stream!!!";

  std::iostream & test_stream_orig = sm.GetIOStream("test_stream");
  std::string out_str = "";
  test_stream_orig >> out_str;
  REQUIRE( out_str == "abc123def" );

  std::istream & in_stream = sm.AddInputStream("in_stream");
  std::ostream & out_stream = sm.AddOutputStream("out_stream");
}
