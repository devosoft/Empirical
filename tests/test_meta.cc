#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include <sstream>
#include <string>

#include "base/array.h"
#include "base/vector.h"
#include "base/Ptr.h"
#include "tools/tuple_utils.h"

#include "meta/meta.h"
#include "meta/reflection.h"
#include "meta/TypeID.h"
#include "meta/TypePack.h"
#include "meta/type_traits.h"


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
  // Test GetTypeValue
  size_t int_value = emp::GetTypeValue<int>();
  size_t char_value = emp::GetTypeValue<char>();
  size_t str_value = emp::GetTypeValue<std::string>();
  size_t int_value2 = emp::GetTypeValue<int>();
  size_t bool_value = emp::GetTypeValue<bool>();

  // Make sure that we are generating unique values for types.
  REQUIRE(int_value != char_value);
  REQUIRE(int_value != str_value);
  REQUIRE(int_value != bool_value);
  REQUIRE(char_value != str_value);
  REQUIRE(char_value != bool_value);
  REQUIRE(str_value != bool_value);

  // Make sure that repeated calls for the SAME type do generate the same value.
  REQUIRE (int_value == int_value2);

  // Check TypeID strings...
  REQUIRE(emp::TypeID<char>::GetName() == "char");
  REQUIRE(emp::TypeID<void>::GetName() == "void");
  REQUIRE(emp::TypeID<int>::GetName() == "int32_t");
  REQUIRE(emp::TypeID<std::string>::GetName() == "std::string");

  REQUIRE((emp::TypeID<emp::array<double,7>>::GetName()) == ("emp::array<double,7>"));
  REQUIRE(emp::TypeID<emp::vector<double>>::GetName() == "emp::vector<double>");

  REQUIRE(emp::TypeID<char*>::GetName() == "char*");

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
  test3_t::apply<emp::vector> v;
  v.push_back(1);
  v.push_back(2);
  REQUIRE(v.size() == 2);

  // If reverse works correctly, str will be a string.
  test_t::reverse::get<3> str("It worked!");
  REQUIRE(str == "It worked!");

  // Let's try filtering!
  using test_filtered = test_t::filter<std::is_integral>;
  REQUIRE(test_filtered::GetSize() == 2);

  using test_filtered_out = test_t::filter_out<std::is_integral>;
  REQUIRE(test_filtered_out::GetSize() == 3);

  using test_remove = test_t::remove_t<std::string>;
  REQUIRE(test_remove::GetSize() == 4);

  using test_A = emp::TypePack<HasA, std::string, bool, HasA2, HasA, int>;
  REQUIRE(emp::TypeID<test_A>::GetName() == "emp::TypePack<HasA,std::string,bool,HasA2,HasA,int32_t>");

  using test_exist = test_A::filter<MemberA>;
  REQUIRE(test_exist::GetSize() == 3);

  using test_print = test_exist::set<1,int>;
  REQUIRE(test_print::Count<int>() == 1);

  using wrap_v_t = test_t::wrap<emp::vector>;
  REQUIRE(emp::TypeID<wrap_v_t>::GetName() == "emp::TypePack<emp::vector<int32_t>,emp::vector<std::string>,emp::vector<float>,emp::vector<bool>,emp::vector<double>>");

  using wrap_A_t = test_A::wrap<MemberA>;
  REQUIRE(emp::TypeID<wrap_A_t>::GetName() == "emp::TypePack<int32_t,char,int32_t>");

  using shuffle_t = test_t::select<2,3,4,1,3,3,3,0>;
  REQUIRE(emp::TypeID<shuffle_t>::GetName() == "emp::TypePack<float,bool,double,std::string,bool,bool,bool,int32_t>");


  using dup_test_t = emp::TypePack<int, int, double, int, double, std::string, bool, int, char, int>;
  REQUIRE(dup_test_t::GetSize() == 10);
  REQUIRE(dup_test_t::remove_t<int>::GetSize() == 5);
  REQUIRE(dup_test_t::remove_t<double>::GetSize() == 8);
  REQUIRE(dup_test_t::make_unique::GetSize() == 5);
  REQUIRE(dup_test_t::Count<int>() == 5);
  REQUIRE(dup_test_t::CountUnique() == 5);

  using link1_t = emp::TypePack<bool, char, int>;
  using link2_t = emp::TypePack<double, int, size_t>;
  using merge_t = link1_t::merge<link2_t>;
  using union_t = link1_t::find_union<link2_t>;
  REQUIRE(link1_t::GetSize() == 3);
  REQUIRE(link2_t::GetSize() == 3);
  REQUIRE(merge_t::GetSize() == 6);
  REQUIRE(union_t::GetSize() == 5);
}

TEST_CASE("Test type traits", "[meta]") {
  REQUIRE((emp::test_type<std::is_pointer, emp::remove_ptr_type_t<int*>>()) == false);
  REQUIRE((emp::test_type<emp::is_ptr_type, emp::remove_ptr_type_t<int*>>()) == false);
  REQUIRE((emp::test_type<std::is_pointer, emp::remove_ptr_type_t<emp::Ptr<int>>>()) == false);
  REQUIRE((emp::test_type<emp::is_ptr_type, emp::remove_ptr_type_t<emp::Ptr<int>>>()) == false);
  REQUIRE((emp::test_type<std::is_pointer, emp::remove_ptr_type_t<int>>()) == false);
  REQUIRE((emp::test_type<emp::is_ptr_type, emp::remove_ptr_type_t<int>>()) == false);

  REQUIRE((emp::test_type<emp::is_ptr_type, int*>()) == true);
  REQUIRE((emp::test_type<emp::is_ptr_type, emp::Ptr<int>>()) == true);

  REQUIRE((std::is_same<int, emp::remove_ptr_type_t<int*>>()) == true);
  REQUIRE((std::is_same<int, emp::remove_ptr_type_t<emp::Ptr<int>>>()) == true);

}
struct Base {};
struct Derived : Base {};
struct Orthogonal {};

TEST_CASE("Test Variadics", "[meta]") {
  REQUIRE(
    (!emp::variadic_contains<int,
                             std::tuple<float, char, unsigned int>>::value));
  REQUIRE((
    emp::variadic_contains<int,
                           std::tuple<float, int, char, unsigned int>>::value));

  // Check that classes with non-variadic numbers of arguments are supported
  REQUIRE((emp::variadic_contains<int, std::pair<float, int>>::value));

  // Check that alternative equality operators are supported
  REQUIRE(
    (!emp::variadic_contains<Base, std::pair<Derived, Orthogonal>>::value));
  REQUIRE((emp::variadic_contains<Base, std::pair<Derived, Orthogonal>,
                                  std::is_base_of>::value));
  REQUIRE((!emp::variadic_contains<Derived, std::pair<Base, Orthogonal>,
                                   std::is_base_of>::value));

  // Check concat
  REQUIRE((std::is_same<
           emp::variadic_concat<std::tuple<int, float, char>,
                                std::tuple<short, unsigned int, char*>>::type,
           std::tuple<int, float, char, short, unsigned int, char*>>::value));

  // Check filter
  REQUIRE((std::is_same<
           emp::variadic_filter_t<std::tuple<int, float, long, float, char*>,
                                  std::is_integral>,
           std::tuple<int, long>>::value));

  // Check remove duplicates
  REQUIRE(
    (std::is_same<
      emp::variadic_remove_duplicates_t<std::tuple<int, float, float, char>>,
      std::tuple<int, float, char>>::value));
  REQUIRE((std::is_same<emp::variadic_remove_duplicates_t<
                          std::tuple<int, int, float, float, char>>,
                        std::tuple<int, float, char>>::value));
  REQUIRE((std::is_same<
           emp::variadic_remove_duplicates_t<std::tuple<int, float, char>>,
           std::tuple<int, float, char>>::value));
  REQUIRE((std::is_same<emp::variadic_remove_duplicates_t<std::tuple<int, int>>,
                        std::tuple<int>>::value));
  REQUIRE((std::is_same<emp::variadic_remove_duplicates_t<std::tuple<int>>,
                        std::tuple<int>>::value));
  REQUIRE((std::is_same<emp::variadic_remove_duplicates_t<std::tuple<>>,
                        std::tuple<>>::value));
  REQUIRE(
    (std::is_same<emp::variadic_remove_duplicates_t<
                    std::tuple<Base, Derived, Orthogonal>, std::is_convertible>,
                  std::tuple<Base, Orthogonal>>::value));

  // Test Union
  REQUIRE((std::is_same<
           emp::variadic_union_t<std::is_same, std::tuple<int, float, char>,
                                 std::tuple<int, char*, int*>>,
           std::tuple<int, float, char, char*, int*>>::value));
  REQUIRE((std::is_same<
           emp::variadic_union_t<std::is_same, std::tuple<int, float, char>,
                                 std::tuple<int, char*, int*>,
                                 std::tuple<int*, char**, int**>>,
           std::tuple<int, float, char, char*, int*, char**, int**>>::value));
  REQUIRE(
    (std::is_same<emp::variadic_union_t<std::is_same, std::pair<int, float>,
                                        std::pair<int, float>>,
                  std::pair<int, float>>::value));
  REQUIRE((std::is_same<emp::variadic_union_t<std::is_same, std::tuple<>,
                                              std::tuple<int, char*, int*>>,
                        std::tuple<int, char*, int*>>::value));

  // Test Intersection
  REQUIRE((std::is_same<emp::variadic_intersection_t<
                          std::is_same, std::tuple<int, float, float, char>,
                          std::tuple<int, char*, int*, int>>,
                        std::tuple<int>>::value));
  REQUIRE((std::is_same<emp::variadic_intersection_t<
                          std::is_same, std::tuple<int, float, float, char>,
                          std::tuple<int, char*, int*, int>, std::tuple<int>>,
                        std::tuple<int>>::value));
  REQUIRE(
    (std::is_same<emp::variadic_union_t<std::is_same, std::pair<int, float>,
                                        std::pair<int, float>>,
                  std::pair<int, float>>::value));
  REQUIRE((std::is_same<
           emp::variadic_intersection_t<std::is_same, std::tuple<>,
                                        std::tuple<int, char*, int*, int>>,
           std::tuple<>>::value));
}
