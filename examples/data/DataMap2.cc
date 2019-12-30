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

  
  // -------------- Examples of EXTERNAL images --------------

  emp::DataMap_Key data_map;

  const auto & default_image = data_map.GetDefaultImage();

  std::cout << "\nAt start:\n  default_image.GetSize() == " << data_map.GetImageSize()
            << std::endl;

  // Add some values...

  size_t idA = data_map.Add<int>("test_int", 20);
  size_t idB = data_map.Add<double>("test_double", 111.111);
  size_t idC = data_map.Add<std::string>("short_string", "string1");
  size_t idD = data_map.Add<std::string>("long_string", "This is a much longer string that shouldn't be used for short-string optimization.");

  std::cout << "\nAfter inserting an int, a double, and two strings:"
            << "\n  default_image.GetSize() == " << data_map.GetImageSize()
            << "\n  (A) Default: " << default_image.Get<int>(idA)
            << "\n  (B) Default: " << default_image.Get<double>(idB)
            << "\n  (C) Default: " << default_image.Get<std::string>(idC)
            << "\n  (D) Default: " << default_image.Get<std::string>(idD)
            << std::endl;

  emp::DataMap image1(data_map);

  std::cout << "\nAfter initializing image 1:"
            << "\n  image1.GetSize() == " << image1.GetSize()
            << "\n  default_image.GetSize() == " << data_map.GetImageSize()
            << "\n  (A) Default: " << default_image.Get<int>(idA)
            << "\n      Image1 : " << image1.Get<int>(idA)
            << "\n  (B) Default: " << default_image.Get<double>(idB)
            << "\n      Image1 : " << image1.Get<double>(idB)
            << "\n  (C) Default: " << default_image.Get<std::string>(idC)
            << "\n      Image1 : " << image1.Get<std::string>(idC)
            << "\n  (D) Default: " << default_image.Get<std::string>(idD)
            << "\n      Image1 : " << image1.Get<std::string>(idD)
            << std::endl;

  data_map.GetDefault<double>(idB) = 222.222;

  std::cout << "\nAfter changing the default double value to 222.222:"
            << "\n  image1.GetSize() == " << image1.GetSize()
            << "\n  default_image.GetSize() == " << data_map.GetImageSize()
            << "\n  (A) Default: " << default_image.Get<int>(idA)
            << "\n      Image1 : " << image1.Get<int>(idA)
            << "\n  (B) Default: " << default_image.Get<double>(idB)
            << "\n      Image1 : " << image1.Get<double>(idB)
            << "\n  (C) Default: " << default_image.Get<std::string>(idC)
            << "\n      Image1 : " << image1.Get<std::string>(idC)
            << "\n  (D) Default: " << default_image.Get<std::string>(idD)
            << "\n      Image1 : " << image1.Get<std::string>(idD)
            << std::endl;

  image1.Get<std::string>(idC)[6] = '2';

  std::cout << "\nAfter changing the image1 short-string value to 'string2':"
            << "\n  image1.GetSize() == " << image1.GetSize()
            << "\n  default_image.GetSize() == " << data_map.GetImageSize()
            << "\n  (A) Default: " << default_image.Get<int>(idA)
            << "\n      Image1 : " << image1.Get<int>(idA)
            << "\n  (B) Default: " << default_image.Get<double>(idB)
            << "\n      Image1 : " << image1.Get<double>(idB)
            << "\n  (C) Default: " << default_image.Get<std::string>(idC)
            << "\n      Image1 : " << image1.Get<std::string>(idC)
            << "\n  (D) Default: " << default_image.Get<std::string>(idD)
            << "\n      Image1 : " << image1.Get<std::string>(idD)
            << std::endl;


  emp::DataMap image2(data_map);

  std::cout << "\nAfter initializing image2 with current defaults:"
            << "\n  image1.GetSize() == " << image1.GetSize()
            << "\n  image2.GetSize() == " << image2.GetSize()
            << "\n  default_image.GetSize() == " << data_map.GetImageSize()
            << "\n  (A) Default: " << default_image.Get<int>(idA)
            << "\n      Image1 : " << image1.Get<int>(idA)
            << "\n      Image2 : " << image2.Get<int>(idA)
            << "\n  (B) Default: " << default_image.Get<double>(idB)
            << "\n      Image1 : " << image1.Get<double>(idB)
            << "\n      Image2 : " << image2.Get<double>(idB)
            << "\n  (C) Default: " << default_image.Get<std::string>(idC)
            << "\n      Image1 : " << image1.Get<std::string>(idC)
            << "\n      Image2 : " << image2.Get<std::string>(idC)
            << "\n  (D) Default: " << default_image.Get<std::string>(idD)
            << "\n      Image1 : " << image1.Get<std::string>(idD)
            << "\n      Image2 : " << image2.Get<std::string>(idD)
            << std::endl;

  image1.Get<std::string>(idD)[6] = '2';

  std::cout << "\nAfter changing the image1 LONG-string value to have a '2':"
            << "\n  image1.GetSize() == " << image1.GetSize()
            << "\n  image2.GetSize() == " << image2.GetSize()
            << "\n  default_image.GetSize() == " << data_map.GetImageSize()
            << "\n  (A) Default: " << default_image.Get<int>(idA)
            << "\n      Image1 : " << image1.Get<int>(idA)
            << "\n      Image2 : " << image2.Get<int>(idA)
            << "\n  (B) Default: " << default_image.Get<double>(idB)
            << "\n      Image1 : " << image1.Get<double>(idB)
            << "\n      Image2 : " << image2.Get<double>(idB)
            << "\n  (C) Default: " << default_image.Get<std::string>(idC)
            << "\n      Image1 : " << image1.Get<std::string>(idC)
            << "\n      Image2 : " << image2.Get<std::string>(idC)
            << "\n  (D) Default: " << default_image.Get<std::string>(idD)
            << "\n      Image1 : " << image1.Get<std::string>(idD)
            << "\n      Image2 : " << image2.Get<std::string>(idD)
            << std::endl;

}
