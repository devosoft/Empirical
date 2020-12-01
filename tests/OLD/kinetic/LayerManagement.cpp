// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <emscripten.h>
#include <iostream>
#include <cmath>

#include "kinetic/Kinetic.h"

class KineticExample {
private:
  emp::Kinetic::Stage stage;
  emp::Kinetic::Layer layer_anim;
  emp::Kinetic::Layer layer_static;

  emp::Kinetic::RegularPolygon blue_hex;
  emp::Kinetic::RegularPolygon yellow_hex;
  emp::Kinetic::RegularPolygon red_hex;
  emp::Kinetic::TextBox text;

  emp::Kinetic::Animation<KineticExample> anim;


public:
  KineticExample()
    : stage(578, 200, "container")
    , blue_hex(100, stage.GetHeight()/2, 7, 70, "#00D2FF", "black", 10, true)
    , yellow_hex(stage.GetWidth()/2, stage.GetHeight()/2, 6, 70, "yellow", "red", 4, true)
    , red_hex(470, stage.GetHeight()/2, 6, 70, "red", "black", 4, true)
    , text(10, 10, "Static Layer C++", 30, "Calibri", "black")
  {
    layer_anim.Add(blue_hex);
    layer_anim.Add(yellow_hex);
    layer_anim.Add(red_hex);
    layer_static.Add(text);
    red_hex.SetOffset(70, 0);
    stage.Add(layer_anim).Add(layer_static);

    anim.Setup(this, &KineticExample::Animate, layer_anim);
    anim.Start();
  }

  void Animate(const emp::Kinetic::AnimationFrame & frame) {
    const double PI = 3.141592653589793238463;
    const double period = 2000.0;
    const double scale = std::sin(frame.time * 2 * PI / period) + 0.001;

    blue_hex.SetScale(scale, scale);    // scale x and y
    yellow_hex.SetScale(1.0, scale);    // scale only x
    red_hex.SetScale(scale, 1.0);       // scale only y
  }
};



KineticExample * example;

extern "C" int empMain()
{
  example = new KineticExample();

  return 0;
}
