//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "base/vector.h"

int main()
{
  emp::vector<int> v;
  for (int i = 0; i < 10; i++) {
    v.push_back( 10*i );
  }

  v.insert(v.begin() + 3, -1);

  for (auto x : v) {
    std::cout << x << ' ';
  }
  std::cout << std::endl;
}
