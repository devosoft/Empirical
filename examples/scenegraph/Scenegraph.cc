//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//

#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>

#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/glcanvas.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "scenegraph/shapes.h"
#include "scenegraph/transform.h"

#include <chrono>
#include <cstdlib>

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::math;
  using namespace emp::scenegraph;
  using namespace emp::scenegraph::shapes;

  GLCanvas canvas;

  Region3f region{{-100, -100, -100}, {100, 100, 100}};

  Stage stage(region);

  // stage.MakeRoot<Group>()->Attach(
  //   std::make_shared<Transform<Rectangle>>(canvas, 1.f, Color::red()));

  stage.MakeRoot<Group>()->Fill(100, [&canvas](auto...) {
    auto r =
      std::make_shared<Transform<Rectangle>>(canvas, 5.f, Color::red(1, 0.5));

    r->translate(rand() % 50 - 25, rand() % 50 - 25, 0);

    return r;
  });

  OrthoCamera camera(region);

  // PerspectiveCamera camera(consts::pi<float> / 4,
  //                          canvas.getWidth() / (float)canvas.getHeight(),
  //                          0.1, 100);

  SimpleEye eye;
  eye.LookAt({40, 30, 30}, {0, 0, 0}, {0, 0, -1});
  // eye.LookAt({10, 10, 10}, {0, 1, 0}, {0, 0, 0});

  // canvas.on_mouse_event.bind(
  //   [&camera, &depth, &eye](auto&, const MouseEvent& event) {
  //     // if (event.button.Clicked()) {
  //     //   if (event.button.button == MouseEvent::Button::Left) {
  //     //     camera.next();
  //     //   } else {
  //     //     ++depth;
  //     //     eye.LookAt({0, 0, depth}, {0, 1, 0}, {0, 0, 0});
  //     //   }
  //     // }
  //   });

  canvas.runForever([&](auto&&) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    stage.Render(camera, eye);
  });

  return 0;
}
