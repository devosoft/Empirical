/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023-2024.
 *
 *  @file SerialPod.cpp
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/serialize/SerialPod.hpp"

enum class TestEnum { ONE=1, TWO=2, THREE=3, TEN=10 };

TEST_CASE("Test SerialPod with simple types", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // Simple saving and loading.
  int in1 = 91;
  std::string in2 = "Test String";
  char in3 = '%';

  save_pod(in1, in2, in3);

  // std::cout << "------" << std::endl << ss.str() << std::endl << "------" << std::endl;

  int out1 = 0;
  std::string out2 = "Not the original.";
  char out3 = ' ';

  load_pod(out1, out2, out3);

  CHECK(in1 == out1);
  CHECK(in2 == out2);
  CHECK(in3 == out3);
}

TEST_CASE("Test SerialPod with nuanced types", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // Test enumerations
  TestEnum in4 = TestEnum::TWO;
  TestEnum in5 = TestEnum::TEN;

  save_pod(in4, in5);

  TestEnum out4, out5;

  load_pod(out4, out5);

  CHECK(in4 == out4);
  CHECK(in5 == out5);

  // Try saving a const value; for the moment load it as non-const.
  const int const_val = 42;
  save_pod(const_val);

  int load_val = 0;

  CHECK(const_val != load_val);
  load_pod(load_val);
  CHECK(const_val == load_val);

  // Try saving a temporary value.
  save_pod(const_val / 2);
  load_pod(load_val);
  CHECK(load_val == 21);
}

struct Struct_Internal_Serialize {
  int x = 0;
  int y = 0;
  unsigned long long z = 0;

  void Serialize(emp::SerialPod & pod) { pod(x, y, z); }

  bool operator==(const Struct_Internal_Serialize &) const = default;
};

struct Struct_External_Serialize {
  uint64_t x = 0;
  uint16_t y = 0;
  uint32_t z = 0;

  bool operator==(const Struct_External_Serialize &) const = default;
};

namespace emp {
  void Serialize(emp::SerialPod & pod, Struct_External_Serialize & in) { pod(in.x, in.y, in.z); }
}

struct Struct_Internal_SaveLoad {
  std::string a = "default";
  std::string b = "default";
  std::string c = "default";

  void SerialSave(emp::SerialPod & pod) {
    pod.Save(a);
    pod.Save(b);
    pod.Save(c);
  }

  void SerialLoad(emp::SerialPod & pod) {
    pod.Load(a);
    pod.Load(b);
    pod.Load(c);
  }

  bool operator==(const Struct_Internal_SaveLoad &) const = default;
};

struct Struct_External_SaveLoad {
  std::string a = "default";
  std::string b = "default";
  std::string c = "default";
  std::string d = "extra default";

  bool operator==(const Struct_External_SaveLoad &) const = default;
};

void SerialSave(emp::SerialPod & pod, Struct_External_SaveLoad & in) {
  pod.Save(in.a);
  pod.Save(in.b);
  pod.Save(in.c);
  pod.Save(in.d);
}

void SerialLoad(emp::SerialPod & pod, Struct_External_SaveLoad & in) {
  pod.Load(in.a);
  pod.Load(in.b);
  pod.Load(in.c);
  pod.Load(in.d);
}

TEST_CASE("Test SerialPod with simple custom classes", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // Test custom class with Serialize member
  Struct_Internal_Serialize in1{5, 50, 5000000};
  Struct_Internal_Serialize in2{6, 77, 888888888};

  save_pod(in1, in2);

  Struct_Internal_Serialize out1, out2;

  load_pod(out1, out2);

  CHECK(in1 == out1);
  CHECK(in2 == out2);

  // Test custom class with external Serialize

  Struct_External_Serialize in3{5000000000ul, 50, 5000000};
  Struct_External_Serialize in4{6, 77, 88888};

  // std::cout << ss.str() << std::endl;

  save_pod(in3, in4);

  Struct_External_Serialize out3, out4;

  load_pod(out3, out4);

  CHECK(in3 == out3);
  CHECK(in4 == out4);

  // Test custom class with SerialLoad and SerialSave members

  Struct_Internal_SaveLoad in5{"one", "two", "three"};
  Struct_Internal_SaveLoad in6{"aaa", "bb", "c"};

  // std::cout << ss.str() << std::endl;

  save_pod(in5, in6);

  Struct_Internal_SaveLoad out5, out6;

  load_pod(out5, out6);

  CHECK(in5 == out5);
  CHECK(in6 == out6);

  // Test custom class with external SerialLoad and SerialSave

  Struct_External_SaveLoad in7{"four", "five", "six", "seven"};
  Struct_External_SaveLoad in8{"add", "beep", "circle", "digraph"};

  // std::cout << ss.str() << std::endl;

  save_pod(in7, in8);

  Struct_External_SaveLoad out7, out8;

  load_pod(out7, out8);

  CHECK(in7 == out7);
  CHECK(in8 == out8);
}


struct Struct_Nested {
  Struct_Internal_Serialize s1;
  Struct_External_Serialize s2;
  std::string name = "none";

  bool operator==(const Struct_Nested &) const = default;

  void Serialize(emp::SerialPod & pod) { pod(s1, s2, name); }
};



TEST_CASE("Test SerialPod with more complex classes", "[serialize]")
{
  // Test nested custom classes.
  Struct_Nested in;
  in.s1.x = 50;
  in.s1.y = -100;
  in.s1.z = 2;
  in.s2.x = 10;
  in.s2.y = 11;
  in.s2.z = 12;

  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  save_pod(in);

  Struct_Nested out;

  load_pod(out);

  CHECK(in == out);

  CHECK(out.s1.x == 50);
  CHECK(out.s1.y == -100);
  CHECK(out.s1.z == 2);
  CHECK(out.s2.x == 10);
  CHECK(out.s2.y == 11);
  CHECK(out.s2.z == 12);
}

TEST_CASE("Test SerialPod with standard library containers", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  std::vector<int> vec1{1,2,3,4,5,6,-10}, vec2;

  CHECK(vec1.size() == 7);
  CHECK(vec2.size() == 0);

  save_pod(vec1);
  load_pod(vec2);
  CHECK(vec1.size() == 7);
  CHECK(vec2.size() == 7);
  CHECK(vec2[0] == 1);
  CHECK(vec2[6] == -10);

  // Loading into a non-empty vector must replace, not append.
  std::vector<int> vec3{100, 200, 300};
  std::vector<int> vec4{1, 2, 3, 4, 5};

  save_pod(vec3);
  load_pod(vec4);  // vec4 had 5 elements; should be replaced by vec3's 3
  CHECK(vec4.size() == 3);
  CHECK(vec4[0] == 100);
  CHECK(vec4[2] == 300);
}

TEST_CASE("Test SerialPod with set-like containers", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // std::set: elements are stored in sorted order; duplicates discarded on insert.
  std::set<int> set1{3, 1, 4, 1, 5, 9, 2, 6};  // unique: {1,2,3,4,5,6,9}
  std::set<int> set2;

  save_pod(set1);
  load_pod(set2);

  CHECK(set1 == set2);
  CHECK(set2.size() == 7);
  CHECK(set2.count(9) == 1);
  CHECK(set2.count(7) == 0);

  // Loading into a non-empty set replaces its contents.
  std::set<int> set3{10, 20, 30};
  std::set<int> set4{1, 2, 3, 4, 5};

  save_pod(set3);
  load_pod(set4);

  CHECK(set4 == set3);
  CHECK(set4.size() == 3);

  // std::unordered_set: order of iteration is not guaranteed, but equality works.
  std::unordered_set<int> uset1{10, 20, 30, 40, 50};
  std::unordered_set<int> uset2;

  save_pod(uset1);
  load_pod(uset2);

  CHECK(uset1 == uset2);
  CHECK(uset2.size() == 5);
  CHECK(uset2.count(30) == 1);
  CHECK(uset2.count(99) == 0);
}

TEST_CASE("Test SerialPod with map-like containers", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // std::map with string keys and int values.
  std::map<std::string, int> map1{{"alpha", 1}, {"beta", 2}, {"gamma", 3}};
  std::map<std::string, int> map2;

  save_pod(map1);
  load_pod(map2);

  CHECK(map1 == map2);
  CHECK(map2.size() == 3);
  CHECK(map2["alpha"] == 1);
  CHECK(map2["gamma"] == 3);

  // Loading into a non-empty map replaces its contents.
  std::map<int, int> map3{{1, 10}, {2, 20}};
  std::map<int, int> map4{{5, 50}, {6, 60}, {7, 70}};

  save_pod(map3);
  load_pod(map4);

  CHECK(map4 == map3);
  CHECK(map4.size() == 2);
  CHECK(map4.count(5) == 0);  // old entries gone

  // std::unordered_map: order of iteration is not guaranteed, but equality works.
  std::unordered_map<int, std::string> umap1{{1, "one"}, {2, "two"}, {3, "three"}};
  std::unordered_map<int, std::string> umap2;

  save_pod(umap1);
  load_pod(umap2);

  CHECK(umap1 == umap2);
  CHECK(umap2.size() == 3);
  CHECK(umap2[1] == "one");
  CHECK(umap2[3] == "three");
}

TEST_CASE("Test SerialPod with std::pair", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  std::pair<int, std::string> p1{42, "hello"};
  std::pair<int, std::string> p2{0, ""};

  save_pod(p1);
  load_pod(p2);

  CHECK(p2.first  == 42);
  CHECK(p2.second == "hello");

  // Pair nested inside a vector.
  std::vector<std::pair<int, int>> vp1{{1, 10}, {2, 20}, {3, 30}};
  std::vector<std::pair<int, int>> vp2;

  save_pod(vp1);
  load_pod(vp2);

  CHECK(vp1 == vp2);
}

TEST_CASE("Test SerialPod with std::optional", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // With value present.
  std::optional<int> a{99};
  std::optional<int> b;

  save_pod(a);
  load_pod(b);

  CHECK(b.has_value());
  CHECK(*b == 99);

  // Without value (nullopt).
  std::optional<int> c;
  std::optional<int> d{42};  // should be cleared on load

  save_pod(c);
  load_pod(d);

  CHECK(!d.has_value());

  // String optional.
  std::optional<std::string> e{"world"};
  std::optional<std::string> f;

  save_pod(e);
  load_pod(f);

  CHECK(f.has_value());
  CHECK(*f == "world");
}

TEST_CASE("Test SerialPod with std::array", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  std::array<int, 5> arr1{10, 20, 30, 40, 50};
  std::array<int, 5> arr2{};

  save_pod(arr1);
  load_pod(arr2);

  CHECK(arr1 == arr2);

  // Array of strings.
  std::array<std::string, 3> sarr1{"alpha", "beta", "gamma"};
  std::array<std::string, 3> sarr2{};

  save_pod(sarr1);
  load_pod(sarr2);

  CHECK(sarr1 == sarr2);
}

TEST_CASE("Test SerialPod with std::tuple", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // Mixed-type tuple.
  std::tuple<int, std::string, double> t1{7, "tuple", 3.14};
  std::tuple<int, std::string, double> t2{0, "", 0.0};

  save_pod(t1);
  load_pod(t2);

  CHECK(std::get<0>(t2) == 7);
  CHECK(std::get<1>(t2) == "tuple");
  CHECK(std::get<2>(t2) == 3.14);

  // Single-element tuple.
  std::tuple<int> t3{55};
  std::tuple<int> t4{0};

  save_pod(t3);
  load_pod(t4);

  CHECK(std::get<0>(t4) == 55);
}

TEST_CASE("Test SerialPod with std::variant", "[serialize]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  using V = std::variant<int, std::string, double>;

  // Active alternative: int.
  V v1{42};
  V v2{0};

  save_pod(v1);
  load_pod(v2);

  CHECK(v2.index() == 0);
  CHECK(std::get<int>(v2) == 42);

  // Active alternative: string.
  V v3{std::string{"variant"}};
  V v4{0};

  save_pod(v3);
  load_pod(v4);

  CHECK(v4.index() == 1);
  CHECK(std::get<std::string>(v4) == "variant");

  // Active alternative: double.
  V v5{2.718};
  V v6{0};

  save_pod(v5);
  load_pod(v6);

  CHECK(v6.index() == 2);
  CHECK(std::get<double>(v6) == 2.718);

  // Variant with monostate.
  using MV = std::variant<std::monostate, int, std::string>;

  MV mv1{std::monostate{}};
  MV mv2{99};

  save_pod(mv1);
  load_pod(mv2);

  CHECK(mv2.index() == 0);
  CHECK(std::holds_alternative<std::monostate>(mv2));
}

TEST_CASE("Test SerialPod with const creation from constructor", "[serialize]")
{
}

TEST_CASE("Test SerialPod with pointer management", "[serialize]")
{
}

TEST_CASE("Test SerialPod with pointer linkage", "[serialize]")
{
}

TEST_CASE("Test SerialPod with custom linked list", "[serialize]")
{
}

// Local settings for Empecable file checker.
// empecable_words: aaa
