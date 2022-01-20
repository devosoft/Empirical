/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file IndexMap.cpp
 */

#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/datastructs/IndexMap.hpp"
#include "emp/math/Random.hpp"

int main()
{
  const int SET_SIZE = 10000;

  emp::IndexMap test_imap(SET_SIZE);
  test_imap[0] = 0.0;
  test_imap[1] = 0.5;
  test_imap[2] = 1.0;
  test_imap[3] = 1.5;
  test_imap[4] = 2.0;
  test_imap[5] = 2.5;
  test_imap[6] = 3.0;
  test_imap[7] = 3.5;
  test_imap[8] = 4.0;
  test_imap[9] = 4.5;
  std::cout << "Ping!" << std::endl;

  emp::Random random;
  emp::vector<int> count((size_t)10,(int)0);
  for (size_t i = 0; i < 45000; i++) {
    size_t id = test_imap.Index(random.GetDouble(0,test_imap.GetWeight()));
    count[id]++;
  }
  for (size_t id = 0; id < 10; id++) {
    std::cout << id << " : " << count[id] << std::endl;
  }

  std::cout << "Start weight = " << test_imap.GetWeight() << std::endl;

  std::cout << "\n... adjusting set weights ...\n";
  for (size_t i = 0; i < 100000000; i++) {
    test_imap[random.GetUInt(SET_SIZE)] = random.GetDouble(0.0,5000.0);
  }

  std::cout << "End weight = " << test_imap.GetWeight() << std::endl;

}
