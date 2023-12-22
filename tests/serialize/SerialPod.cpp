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

TEST_CASE("Test SerialPod", "[control]")
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
}
