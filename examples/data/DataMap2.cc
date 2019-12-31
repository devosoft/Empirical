/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  DataMap.cc
 *  @brief An example file for using DataMaps
 */

#include <iostream>

#include "data/DataMap2.h"

int main()
{
  emp::DataMap dmap;

  size_t id1 = dmap.Add<double>("fitness", 0.0);
  size_t id2 = dmap.Add<bool>("do_muts", false);
  size_t id3 = dmap.Add<std::string>("name", "MyOrg1");
  size_t id4 = dmap.Add<std::string>("name2", "MyOrg2");
  size_t id5 = dmap.Add<std::string>("name3", "MyOrg3");

  std::cout << "IDs: "
            << " id1 = " << id1
            << " id2 = " << id2
            << " id3 = " << id3
            << " id4 = " << id4
            << " id5 = " << id5
            << std::endl;

  dmap.Get<std::string>("name") = "FirstOrg";
  dmap.Get<std::string>(id4) = "Org TWO!";
  dmap.Get<std::string>("name3") = "Test Output!";
  dmap.Get<double>("fitness") = 1000000.1;

  std::cout << "Name 1 = " << dmap.Get<std::string>("name") << std::endl;
  std::cout << "Name 2 = " << dmap.Get<std::string>("name2") << std::endl;
  std::cout << "Name 3 = " << dmap.Get<std::string>("name3") << std::endl;
  std::cout << "Name 3 = " << dmap.Get<std::string>(id5) << std::endl;
  std::cout << "Name 3 = " << dmap.Get<std::string>( dmap.GetID("name3") ) << std::endl;

  std::cout << "\nTypes:\n";
  std::cout << "fitness: " << dmap.GetType("fitness") << std::endl;
  std::cout << "do_muts: " << dmap.GetType("do_muts") << std::endl;
  std::cout << "name:    " << dmap.GetType("name") << std::endl;
  std::cout << "name2:   " << dmap.GetType("name2") << std::endl;
  std::cout << "name3:   " << dmap.GetType("name3") << std::endl;

  
  // -------------- Examples of MULTIPLE DataMaps --------------

  emp::DataMap data_map;

  std::cout << "\nAt start:\n  data_map.GetSize() == " << data_map.GetSize()
            << std::endl;

  // Add some values...

  size_t idA = data_map.Add<int>("test_int", 10);
  size_t idB = data_map.Add<double>("test_double", 111.111);
  size_t idC = data_map.Add<std::string>("short_string", "string1");
  size_t idD = data_map.Add<std::string>("long_string", "This is a much longer string that shouldn't be used for short-string optimization.");

  std::cout << "\nAfter inserting an int, a double, and two strings:"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>(idA)
            << "\n  (B) data_map : " << data_map.Get<double>(idB)
            << "\n  (C) data_map : " << data_map.Get<std::string>(idC)
            << "\n  (D) data_map : " << data_map.Get<std::string>(idD)
            << std::endl;

  std::cout << "\nAnd calling by NAME rather than ID:"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>("test_int")
            << "\n  (B) data_map : " << data_map.Get<double>("test_double")
            << "\n  (C) data_map : " << data_map.Get<std::string>("short_string")
            << "\n  (D) data_map : " << data_map.Get<std::string>("long_string")
            << std::endl;

  emp::DataMap data_map2(data_map);

  std::cout << "\nAfter initializing image 1:"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  data_map2.GetSize() == " << data_map2.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>(idA)
            << "\n      data_map2: " << data_map2.Get<int>(idA)
            << "\n  (B) data_map : " << data_map.Get<double>(idB)
            << "\n      data_map2: " << data_map2.Get<double>(idB)
            << "\n  (C) data_map : " << data_map.Get<std::string>(idC)
            << "\n      data_map2: " << data_map2.Get<std::string>(idC)
            << "\n  (D) data_map : " << data_map.Get<std::string>(idD)
            << "\n      data_map2: " << data_map2.Get<std::string>(idD)
            << std::endl;

  data_map.Set<int>("test_int", 20);
  data_map.Set<double>(idB, 222.222);

  std::cout << "\nAfter changing the test_int to 20 and test_double value to 222.222:"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  data_map2.GetSize() == " << data_map2.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>(idA)
            << "\n      data_map2: " << data_map2.Get<int>(idA)
            << "\n  (B) data_map : " << data_map.Get<double>(idB)
            << "\n      data_map2: " << data_map2.Get<double>(idB)
            << "\n  (C) data_map : " << data_map.Get<std::string>(idC)
            << "\n      data_map2: " << data_map2.Get<std::string>(idC)
            << "\n  (D) data_map : " << data_map.Get<std::string>(idD)
            << "\n      data_map2: " << data_map2.Get<std::string>(idD)
            << std::endl;

  data_map2.Get<std::string>(idC)[6] = '2';

  std::cout << "\nAfter changing the data_map2 short-string value to 'string2':"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  data_map2.GetSize() == " << data_map2.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>(idA)
            << "\n      data_map2: " << data_map2.Get<int>(idA)
            << "\n  (B) data_map : " << data_map.Get<double>(idB)
            << "\n      data_map2: " << data_map2.Get<double>(idB)
            << "\n  (C) data_map : " << data_map.Get<std::string>(idC)
            << "\n      data_map2: " << data_map2.Get<std::string>(idC)
            << "\n  (D) data_map : " << data_map.Get<std::string>(idD)
            << "\n      data_map2: " << data_map2.Get<std::string>(idD)
            << std::endl;


  emp::DataMap data_map3(data_map);

  std::cout << "\nAfter initializing data_map3 with current defaults:"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  data_map2.GetSize() == " << data_map2.GetSize()
            << "\n  data_map3.GetSize() == " << data_map3.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>(idA)
            << "\n      data_map2: " << data_map2.Get<int>(idA)
            << "\n      data_map3: " << data_map3.Get<int>(idA)
            << "\n  (B) data_map : " << data_map.Get<double>(idB)
            << "\n      data_map2: " << data_map2.Get<double>(idB)
            << "\n      data_map3: " << data_map3.Get<double>(idB)
            << "\n  (C) data_map : " << data_map.Get<std::string>(idC)
            << "\n      data_map2: " << data_map2.Get<std::string>(idC)
            << "\n      data_map3: " << data_map3.Get<std::string>(idC)
            << "\n  (D) data_map : " << data_map.Get<std::string>(idD)
            << "\n      data_map2: " << data_map2.Get<std::string>(idD)
            << "\n      data_map3: " << data_map3.Get<std::string>(idD)
            << std::endl;

  data_map2.Get<std::string>(idD)[6] = '2';

  std::cout << "\nAfter changing the data_map2 LONG-string value to have a '2':"
            << "\n  data_map.GetSize() == " << data_map.GetSize()
            << "\n  data_map2.GetSize() == " << data_map2.GetSize()
            << "\n  data_map3.GetSize() == " << data_map3.GetSize()
            << "\n  (A) data_map : " << data_map.Get<int>(idA)
            << "\n      data_map2: " << data_map2.Get<int>(idA)
            << "\n      data_map3: " << data_map3.Get<int>(idA)
            << "\n  (B) data_map : " << data_map.Get<double>(idB)
            << "\n      data_map2: " << data_map2.Get<double>(idB)
            << "\n      data_map3: " << data_map3.Get<double>(idB)
            << "\n  (C) data_map : " << data_map.Get<std::string>(idC)
            << "\n      data_map2: " << data_map2.Get<std::string>(idC)
            << "\n      data_map3: " << data_map3.Get<std::string>(idC)
            << "\n  (D) data_map : " << data_map.Get<std::string>(idD)
            << "\n      data_map2: " << data_map2.Get<std::string>(idD)
            << "\n      data_map3: " << data_map3.Get<std::string>(idD)
            << std::endl;

}
