/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2023
 *
 *  @file SerialPod.cpp
 */

#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/serialize/SerialPod.hpp"

enum class TestEnum { ONE=1, TWO=2, THREE=3, TEN=10 };

TEST_CASE("Test SerialPod with simple types", "[control]")
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

  // Test enumerarations
  TestEnum in4 = TestEnum::TWO;
  TestEnum in5 = TestEnum::TEN;

  save_pod(in4, in5);

  TestEnum out4, out5;

  load_pod(out4, out5);

  CHECK(in4 == out4);
  CHECK(in5 == out5);
}

struct Struct_Simple {
  int x = 0;
  int y = 0;
  unsigned long long z = 0;

  void Serialize(emp::SerialPod & pod) { pod(x, y, z); }

  bool operator==(const Struct_Simple &) const = default;
};

TEST_CASE("Test SerialPod with custom classes", "[control]")
{
  std::stringstream ss;
  emp::SerialPod save_pod(ss, true);
  emp::SerialPod load_pod(ss, false);

  // Test custom class with Serialize member
  Struct_Simple in1{5, 50, 5000000};
  Struct_Simple in2{6, 77, 888888888};

  save_pod(in1, in2);

  Struct_Simple out1, out2;

  load_pod(out1, out2);

  CHECK(in1 == out1);
  CHECK(in2 == out2);

  // Test custom class with external Serialize

  // Test custom class with SerialLoad and SerialSave members

  // Test custom class with external SerialLoad and SerialSave

  // Test nested custom classes.

  // Test const creation from constructor

  // Test pointer management

  // Test pointer linkage

  // Test custome linked list
}
