/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015
 *
 *  @file BatchDraw.cpp
 */

#include <cmath>
#include <emscripten.h>
#include <iostream>

#include "EmKinetic.h"

using namespace std;

class KineticExample {
private:
  emkStage stage;
  emkLayer layer;
  emkRect rect;

  emkAnimation<KineticExample> anim;


public:
  KineticExample()
    : stage(578, 200, "container")
    , rect(289, 100, 200, 20, "green", "black", 4)
  {
    rect.SetOffset(100, 10);
    layer.Add(rect);
    stage.Add(layer);

    stage.On("contentMousemove", this, &KineticExample::DoRectRotation);
  }

  void DoRectRotation() {
    rect.DoRotate(10);
    layer.BatchDraw();
  }
};




extern "C" int emkMain()
{
  KineticExample * example = new KineticExample();
  return (int) example;
}
