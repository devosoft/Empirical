#define CATCH_CONFIG_MAIN
#undef NDEBUG

#include "../third-party/Catch/single_include/catch.hpp"

#include <array>
#include <sstream>
#include <string>

#include "../tools/tuple_utils.h"
#include "../tools/vector.h"

#include "../meta/meta.h"
#include "../meta/reflection.h"
#include "../meta/TypeID.h"
#include "../meta/TypePack.h"


char result_char;
void TestFun(int x, int y, char z) {
  result_char = z + x*y;
}

struct HasA { static int A; static std::string TypeID() { return "HasA"; } };
struct HasA2 { static char A; };
template <typename T> using MemberA = decltype(T::A);

template <typename A, typename B>
struct MetaTestClass { A a; B b; };

int Sum4(int a, int b, int c, int d) { return a+b+c+d; }
namespace emp {
  template<> struct TypeID<HasA2> { static std::string GetName() { return "HasA2"; } };
}



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
}


TEST_CASE("Test reflection", "[meta]")
{
  REQUIRE((emp::test_type<MemberA,int>()) == false);
  REQUIRE((emp::test_type<MemberA,HasA>()) == true);
  REQUIRE((emp::test_type<MemberA,HasA2>()) == true);
  REQUIRE((emp::test_type<std::is_integral,int>()) == true);
  REQUIRE((emp::test_type<std::is_integral,HasA>()) == false);
  REQUIRE((emp::test_type<std::is_integral, HasA2>()) == false);

  // @CAO Need to build more reflection tests, once reflection is restructured.
}

TEST_CASE("Test TypeID", "[meta]")
{
  REQUIRE(emp::TypeID<int>::GetName() == "int32_t");
  REQUIRE(emp::TypeID<char>::GetName() == "char");
  REQUIRE(emp::TypeID<std::string>::GetName() == "std::string");
  REQUIRE(emp::TypeID<std::vector<double>>::GetName() == "std::vector<double,Unknown>");
}

TEST_CASE("Test TypePack", "[meta]")
{
  using test_t = emp::TypePack<int, std::string, float, bool, double>;
  REQUIRE(emp::TypeID<test_t>::GetName() == "emp::TypePack<int32_t,std::string,float,bool,double>");
  REQUIRE(test_t::GetSize() == 5);
  REQUIRE(test_t::GetID<float>() == 2);
  REQUIRE(test_t::add<long long>::GetSize() == 6);
  REQUIRE(test_t::pop::GetID<float>() == 1);

  using test2_t = emp::TypePackFill<int, 4>;
  using test3_t = emp::TypePack<uint64_t>;

  REQUIRE(test2_t::GetSize() == 4);
  REQUIRE(test3_t::GetSize() == 1);

  // Make sure we can use a TypePack to properly set a function type.
  using fun_t = test2_t::to_function_t<int>;
  std::function< fun_t > fun(Sum4);
  REQUIRE(fun(1,2,3,4) == 10);

  using test4_t = test2_t::shrink<2>;
  REQUIRE(test4_t::GetSize() == 2);
  REQUIRE(test_t::merge<test2_t>::GetSize() == 9);

  // IF applied correctly, v will be a vector of uint64_t.
  test3_t::apply<std::vector> v;
  v.push_back(1);
  v.push_back(2);
  REQUIRE(v.size() == 2);

  // If reverse works correctly, str will be a string.
  test_t::reverse::get<3> str("It worked!");
  REQUIRE(str == "It worked!");

  // Let's try filtering!
  using test_filtered = test_t::filter<std::is_integral>;
  REQUIRE(test_filtered::GetSize() == 2);

  using test_A = emp::TypePack<HasA, std::string, bool, HasA2, HasA, int>;
  REQUIRE(emp::TypeID<test_A>::GetName() == "emp::TypePack<HasA,std::string,bool,HasA2,HasA,int32_t>");

  using test_exist = test_A::filter<MemberA>;
  REQUIRE(test_exist::GetSize() == 3);

  using test_print = test_exist::set<1,int>;
  REQUIRE(test_print::Count<int>() == 1);

  using wrap_v_t = test_t::wrap<std::vector>;
  REQUIRE(emp::TypeID<wrap_v_t>::GetName() == "emp::TypePack<std::vector<int32_t,Unknown>,std::vector<std::string,Unknown>,std::vector<float,Unknown>,std::vector<bool,Unknown>,std::vector<double,Unknown>>");
  using wrap_A_t = test_A::wrap<MemberA>;
  REQUIRE(emp::TypeID<wrap_A_t>::GetName() == "emp::TypePack<int32_t,char,int32_t>");

  using shuffle_t = test_t::select<2,3,4,1,3,3,3,0>;
  REQUIRE(emp::TypeID<shuffle_t>::GetName() == "emp::TypePack<float,bool,double,std::string,bool,bool,bool,int32_t>");
}
