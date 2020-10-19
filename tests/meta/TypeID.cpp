#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>

#include "emp/base/array.hpp"
#include "emp/base/vector.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/TypeID.hpp"


struct HasA { static int A; static std::string TypeID() { return "HasA"; } };

TEST_CASE("Test TypeID", "[meta]")
{
  emp::SetupTypeNames();

  // Test GetTypeValue
  size_t int_value = emp::GetTypeID<int>();
  size_t char_value = emp::GetTypeID<char>();
  size_t str_value = emp::GetTypeID<std::string>();
  size_t int_value2 = emp::GetTypeID<int>();
  size_t bool_value = emp::GetTypeID<bool>();

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
  REQUIRE(emp::GetTypeID<char>().GetName() == "char");
  REQUIRE(emp::GetTypeID<void>().GetName() == "void");
  REQUIRE(emp::GetTypeID<int>().GetName() == "int32_t");
  REQUIRE(emp::GetTypeID<std::string>().GetName() == "std::string");

  // Check on qualities.
  REQUIRE(emp::GetTypeID<int>().IsConst() == false);
  REQUIRE(emp::GetTypeID<const int>().IsConst() == true);

  REQUIRE(emp::GetTypeID<char>().IsClass() == false);
  REQUIRE(emp::GetTypeID<std::string>().IsClass() == true);
  REQUIRE(emp::GetTypeID<HasA>().IsClass() == true);

  REQUIRE(emp::GetTypeID<char>().IsReference() == false);
  REQUIRE(emp::GetTypeID<char*>().IsReference() == false);
  REQUIRE(emp::GetTypeID<char&>().IsReference() == true);
  REQUIRE(emp::GetTypeID<emp::Ptr<char>>().IsReference() == false);

  REQUIRE(emp::GetTypeID<char>().IsPointer() == false);
  REQUIRE(emp::GetTypeID<char*>().IsPointer() == true);
  REQUIRE(emp::GetTypeID<char&>().IsPointer() == false);
  REQUIRE(emp::GetTypeID<emp::Ptr<char>>().IsPointer() == true);

    // bool IsArray() { return (info_ptr) ? info_ptr->is_array : false; }
    // bool IsAbstract() { return (info_ptr) ? info_ptr->is_abstract : false; }


  //REQUIRE((emp::GetTypeID<emp::array<double,7>>().GetName()) == ("emp::array<double,7>"));
  //REQUIRE(emp::GetTypeID<emp::vector<double>>().GetName() == "emp::vector<double>");

  REQUIRE(emp::GetTypeID<char*>().GetName() == "char*");
  REQUIRE(emp::GetTypeID<const int>().GetName() == "const int32_t");

  emp::vector<emp::TypeID> type_ids = emp::GetTypeIDs<int, char, int, std::string>();
  REQUIRE(type_ids.size() == 4);
  REQUIRE(type_ids[0].GetID() == type_ids[2].GetID());
  REQUIRE(type_ids[0].GetID() == int_value);
  REQUIRE(type_ids[1].GetID() == char_value);
  REQUIRE(type_ids[3].GetID() == str_value);

  int x = 27;
  std::string s = "27";
  REQUIRE(emp::GetTypeID<int>().ToDouble(&x) == 27.0);
  REQUIRE(std::isnan(emp::GetTypeID<std::string>().ToDouble(&s)));

  REQUIRE(emp::GetTypeID<int>().ToString(&x) == "27");
  REQUIRE(emp::GetTypeID<std::string>().ToString(&s) == "27");

  char c = 'c';
  REQUIRE(emp::GetTypeID<char>().ToString(&c) == "c");

  // Test GetSize()
  REQUIRE(emp::GetTypeID<char>().GetSize() == 1);
}
