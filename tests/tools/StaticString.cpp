/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022-2026
*/
/**
 *  @file
 */

#include "../third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/StaticString.hpp"

TEST_CASE("ShortString comparisons", "[tools]")
{
  emp::ShortString str1 = "ABC";
  CHECK( str1.size() == 3 );
  CHECK( str1[0] == 'A' );
  CHECK( str1[1] == 'B' );
  CHECK( str1[2] == 'C' );

  CHECK( str1 == "ABC" );

  std::string std_str = "DEFGH";

  CHECK( str1 != std_str );
  CHECK( str1 < std_str );
  CHECK( str1 <= std_str );

  CHECK( std_str != str1.AsString() );
  CHECK( std_str > str1.AsString() );
  CHECK( std_str >= str1.AsString() );

  emp::ShortString str2(std_str);

  CHECK( str1 != str2 );
  CHECK( str1 < str2 );
  CHECK( str2 > str1 );
  CHECK( str1 <= str2 );
  CHECK( str2 >= str1 );

  str1 = str2;

  CHECK( str1 == str2 );
  CHECK( str1 <= str2);
  CHECK( str1 >= str2);
}

TEST_CASE("ShortString manipulations", "[tools]")
{
  emp::ShortString str1 = "ABC";
  CHECK(str1 == "ABC");
  str1[0] = 'D';
  CHECK(str1 == "DBC");
  str1[1] = 'E';
  CHECK(str1 == "DEC");
  str1[2] = 'F';
  CHECK(str1 == "DEF");

  str1.push_back('G');
  CHECK(str1 == "DEFG");
  str1.push_back('H').push_back('I');
  CHECK(str1 == "DEFGHI");

  str1 = "test:";
  str1.append("123");
  CHECK(str1 == "test:123");

  std::string str456 = "456";
  str1.append(str456);
  CHECK(str1 == "test:123456");

  emp::ShortString str789 = "789";
  str1.append(str789);
  CHECK(str1 == "test:123456789");
}

TEST_CASE("ShortString empty and capacity", "[tools]")
{
  emp::ShortString s;
  CHECK( s.empty() );
  CHECK( s.size() == 0 );
  CHECK( s.max_size() == 30 );  // ShortString = StaticString<31>: 31 - 1 = 30

  s = "hi";
  CHECK( !s.empty() );
  CHECK( s.size() == 2 );
  CHECK( s.max_size() == 30 );
}

TEST_CASE("ShortString c_str and string_view", "[tools]")
{
  emp::ShortString s = "hello";
  CHECK( std::string(s.c_str()) == "hello" );

  std::string_view sv = s;
  CHECK( sv == "hello" );
  CHECK( sv.size() == 5 );
}

TEST_CASE("ShortString iterators", "[tools]")
{
  emp::ShortString s = "abc";
  std::string out;
  for (char c : s) out += c;
  CHECK( out == "abc" );

  // Verify begin/end span exactly the right range.
  CHECK( s.end() - s.begin() == 3 );
}

TEST_CASE("ShortString resize with filler", "[tools]")
{
  emp::ShortString s = "abc";
  s.resize(6, '-');
  CHECK( s.size() == 6 );
  CHECK( s == "abc---" );

  // Shrink via resize (no filler needed)
  s.resize(2);
  CHECK( s.size() == 2 );
  CHECK( s == "ab" );
}

TEST_CASE("ShortString array-reference construction and assignment", "[tools]")
{
  char buf[] = "hello";                      // char[6] — SIZE=6, copies 5 chars
  emp::ShortString s(buf);
  CHECK( s == "hello" );
  CHECK( s.size() == 5 );

  char buf2[] = "world";
  s = buf2;
  CHECK( s == "world" );

  char buf3[] = "!";
  s.append(buf3);
  CHECK( s == "world!" );
}

TEST_CASE("ShortString string_view comparison and operator std::string", "[tools]")
{
  emp::ShortString s = "test";
  std::string str = s;                       // via operator std::string()
  CHECK( str == "test" );
  CHECK( str.size() == 4 );
}

TEST_CASE("StaticString constexpr", "[tools]")
{
  // These must all be evaluated at compile time.
  // 's' is static so its address is constant — required for constexpr std::string_view.
  static constexpr emp::StaticString<16> s("hello");
  static_assert( s.size()     == 5 );
  static_assert( s.max_size() == 15 );
  static_assert( !s.empty() );
  static_assert( s[0] == 'h' );
  static_assert( s[4] == 'o' );
  static_assert( s == "hello" );
  static_assert( s != "world" );
  static_assert( s <  "zoo" );

  constexpr emp::StaticString<16> s2 = s;   // copy at compile time
  static_assert( s == s2 );

  constexpr auto sv = std::string_view{s};  // conversion at compile time
  static_assert( sv == "hello" );
  static_assert( sv.size() == 5 );

  // push_back and append in constexpr context.
  constexpr auto s3 = []() {
    emp::StaticString<16> t("hi");
    t.push_back('!');
    t.append(" there");
    return t;
  }();
  static_assert( s3 == "hi! there" );
  static_assert( s3.size() == 9 );

  // resize with filler.
  constexpr auto s4 = []() {
    emp::StaticString<16> t("ab");
    t.resize(5, '-');
    return t;
  }();
  static_assert( s4 == "ab---" );
  static_assert( s4.size() == 5 );
}

// Verify StaticString works as a non-type template parameter.
template <emp::StaticString<32> Name>
struct NamedTag {
  static constexpr std::string_view name() { return Name.AsStringView(); }
};

TEST_CASE("StaticString as non-type template parameter", "[tools]")
{
  CHECK( NamedTag<"hello">::name() == "hello" );
  CHECK( NamedTag<"world">::name() == "world" );
  CHECK( NamedTag<"hello">::name() != NamedTag<"world">::name() );

  // Two instantiations with the same string are the same type.
  static_assert( std::is_same_v<NamedTag<"hello">, NamedTag<"hello">> );
  // Two instantiations with different strings are different types.
  static_assert( !std::is_same_v<NamedTag<"hello">, NamedTag<"world">> );
}

TEST_CASE("StaticString different sizes compare", "[tools]")
{
  emp::StaticString<16> s16 = "abc";
  emp::StaticString<32> s32 = "abc";
  CHECK( s16 == s32 );
  CHECK( !(s16 != s32) );

  s32 = "abd";
  CHECK( s16 < s32 );
  CHECK( s32 > s16 );
}
