/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file hash_utils.cpp
 *  @brief Some examples code for using hash_utils.hpp
 */

#include <iostream>
#include <unordered_map>

#include "emp/datastructs/hash_utils.hpp"

int main()
{
  // Test CombineHash()
  std::cout << "\nHash results...:\n";
  std::cout << "hash(2) = " << std::hash<int>()(2) << std::endl
      << "hash(3) = " << std::hash<int>()(3) << std::endl
      << "hash(4) = " << std::hash<int>()(4) << std::endl
      << "CombineHash(4) = " << emp::CombineHash(4) << std::endl
      << "CombineHash(2,3) = " << emp::CombineHash(2,3) << std::endl
      << "CombineHash(2,3) = " << emp::CombineHash(2,3) << std::endl
      << "CombineHash(3,2) = " << emp::CombineHash(3,2) << std::endl
      << "CombineHash(3,4) = " << emp::CombineHash(3,4) << std::endl
      << "CombineHash(2,3,4) = " << emp::CombineHash(2,3,4) << std::endl;
}
