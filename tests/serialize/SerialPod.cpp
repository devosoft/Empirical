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

  // Simple saving and loading.
  int in1 = 91;
  std::string in2 = "Test String";
  char in3 = '%';

  save_pod(in1, in2, in3);

  int out1 = 0;
  std::string out2 = "Not the original.";
  char out3 = ' ';

  emp::SerialPod load_pod(ss, false);
  load_pod(out1, out2, out3);

  CHECK(in1 == out1);
  CHECK(in2 == out2);
  CHECK(in3 == out3);

  // Test enumerations
  TestEnum in4 = TestEnum::TWO;
  TestEnum in5 = TestEnum::TEN;

  save_pod(in4, in5);

  TestEnum out4, out5;

  load_pod(out4, out5);

  CHECK(in4 == out4);
  CHECK(in5 == out5);
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



  // Test standard library containers

  // Test const creation from constructor

  // Test pointer management

  // Test pointer linkage

  // Test custom linked list
}
