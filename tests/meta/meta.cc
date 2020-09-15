#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>

#include "base/array.h"
#include "base/vector.h"
#include "base/Ptr.h"
#include "tools/tuple_utils.h"

#include "meta/meta.h"


char result_char;
void TestFun(int x, int y, char z) {
  result_char = z + (char) (x*y);
}

struct HasA { static int A; static std::string TypeID() { return "HasA"; } };
struct HasA2 { static char A; };
template <typename T> using MemberA = decltype(T::A);

template <typename A, typename B>
struct MetaTestClass { A a; B b; };

int Sum4(int a, int b, int c, int d) { return a+b+c+d; }



TEST_CASE("Test meta-programming helpers (meta.h)", "[meta]")
{
  // TEST FOR VARIADIC HELPER FUNCTIONS:

  REQUIRE((emp::get_type_index<char, char, bool, int, double>()) == 0);
  REQUIRE((emp::get_type_index<int, char, bool, int, double>()) == 2);
  REQUIRE((emp::get_type_index<double, char, bool, int, double>()) == 3);
  REQUIRE((emp::get_type_index<std::string, char, bool, int, double>()) < 0);

  REQUIRE((emp::has_unique_first_type<int, bool, std::string, bool, char>()) == true);
  REQUIRE((emp::has_unique_first_type<bool, int, std::string, bool, char>()) == false);
  REQUIRE((emp::has_unique_types<bool, int, std::string, emp::vector<bool>, char>()) == true);
  REQUIRE((emp::has_unique_types<int, bool, std::string, bool, char>()) == false);


  std::tuple<int, int, char> test_tuple(3,2,'a');
  emp::ApplyTuple(TestFun, test_tuple);

  REQUIRE(result_char == 'g');

  using meta1_t = MetaTestClass<int, double>;
  using meta2_t = emp::AdaptTemplate<meta1_t, char, bool>;
  using meta3_t = emp::AdaptTemplate_Arg1<meta1_t, std::string>;

  meta1_t meta1;
  meta2_t meta2;
  meta3_t meta3;

  meta1.a = (decltype(meta1.a)) 65.5;
  meta1.b = (decltype(meta1.b)) 65.5;
  meta2.a = (decltype(meta2.a)) 65.5;
  meta2.b = (decltype(meta2.b)) 65.5;
  meta3.a = (decltype(meta3.a)) "65.5";
  meta3.b = (decltype(meta3.b)) 65.5;

  REQUIRE( meta1.a == 65 );
  REQUIRE( meta1.b == 65.5 );
  REQUIRE( meta2.a == 'A' );
  REQUIRE( meta2.b == true );
  REQUIRE( meta3.a == "65.5" );
  REQUIRE( meta3.b == 65.5 );

  // Combine hash should always return the original values if only one combined.
  REQUIRE( emp::CombineHash(1) == 1 );
  REQUIRE( emp::CombineHash(2) == std::hash<int>()(2) );
	REQUIRE( emp::CombineHash(3) == std::hash<int>()(3) );
	REQUIRE( emp::CombineHash(4) == std::hash<int>()(4) );
	REQUIRE( emp::CombineHash(2,3) == 0x9e4f79bb );
	REQUIRE( emp::CombineHash(3,2) == 0x9e4779bc);
	REQUIRE( emp::CombineHash(1,2) == 0x9e4779ba);
	REQUIRE( emp::CombineHash(3,4) == 0x9e5779bc);
	REQUIRE( emp::CombineHash(2,3,4) == 0x4f2bc6c1c6c76 );

}
