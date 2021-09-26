/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015
 *
 *  @file Simple.cpp
 */

#include <iostream>

#include <emscripten.h>

// #include "../../kinetic/Kinetic.h"

// class KineticExample {
// private:
//   emp::Kinetic::Stage stage;
//   emp::Kinetic::Layer layer;
//   emp::Kinetic::Rect rect;

// public:
//   KineticExample()
//     : stage(600, 200, "container")
//     , rect(20, 20, 200, 100)
//   {
//     layer.Add(rect);
//     stage.Add(layer);
//   }
// };


// KineticExample * example;

// extern "C" int empMain()
// {
//   example = new KineticExample();

//   return 0;
// }

int main() {
  std::cout << "Testing!!" << std::endl;

  int result = EM_ASM_INT({
      alert("My test: " + $0);
      return $0*2;
    }, 10);

  EM_ASM_INT({
      alert("My test2: " + ($0+2));
    }, result);
}
