/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file MemoryFactory.cpp
 */

#include "../third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/MemoryFactory.hpp"

TEST_CASE("Test MemoryFactory", "[tools]")
{
  emp::MemoryFactory<double> factory;
  factory.Initialize(100, 1000);

  double status = 0.003;
  emp::vector< emp::Ptr<double> > data;
  for (size_t i = 0; i < 900; ++i) {
    data.push_back(factory.Reserve());
    for (size_t j = 0; j < 100; ++j) {
      data[i][j] = status;
      status *= 1.0001;
    }
  }
}
