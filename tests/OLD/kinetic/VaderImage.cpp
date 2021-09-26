/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015
 *
 *  @file VaderImage.cpp
 */

#include <cmath>
#include <iostream>

#include <emscripten.h>
#include <unistd.h>

#include "Kinetic.h"

using namespace std;

class KineticExample {
private:
  emkStage stage;
  emkLayer layer;
  emkRect rect;
  emkRect rect2;
  emkImage image_test;
  emkRegularPolygon poly;
  emkText text;

  emkAnimation<KineticExample> anim;


public:
  KineticExample()
    : stage(600, 600, "container")
    , rect(250, 250, 200, 200, "transparent", "black", 4)
    , rect2(350, 250, 200, 200, "red", "black", 4)
    , image_test("http://www.html5canvastutorials.com/demos/assets/darth-vader.jpg")
    , poly(100, 100, 8, 50, "blue")
    , text(400, 100, "Hello!", "30")
  {
    rect.SetOffset(100, 10);
    rect.SetFillPatternImage(image_test);
    rect2.SetFillPatternImage(image_test);
    layer.Add(rect);
    layer.Add(rect2);
    layer.Add(poly);
    layer.Add(text);
    stage.Add(layer);

    rect.On("mousemove", this, &KineticExample::DoRectRotation);
    rect.On("click", this, &KineticExample::DoClick);
    rect2.On("click", this, &KineticExample::DoClick2);
  }

  void DoClick() {
    poly.SetScale(1.0,1.0);
    layer.BatchDraw();
  }

  void DoClick2() {
    poly.SetScale(2.0,2.0);
    layer.BatchDraw();
  }

  void DoRectRotation() {
    rect.DoRotate(2);
    layer.BatchDraw();
  }

  void DoRectScale() {
    rect.SetScale(0.5, 0.5);
  }
};




extern "C" int emkMain()
{
  KineticExample * example = new KineticExample();
  return (int) example;
}
