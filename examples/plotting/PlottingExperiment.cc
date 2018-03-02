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
#include "plot/line.h"
#include "plot/scales.h"
#include "plot/scatter.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"
// #include "scenegraph/shapes.h"
#include "scenegraph/transform.h"

#include <chrono>
#include <cstdlib>

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::math;
  using namespace emp::scenegraph;
  using namespace emp::plot;
  using namespace emp::plot::attributes;

  GLCanvas canvas;
  Group root;
  auto line{std::make_shared<Line>(canvas)};
  auto scatter{std::make_shared<Scatter>(canvas, 6)};
  auto scale{std::make_shared<Scale<2>>(canvas.getRegion())};
  root.attachAll(scatter, line);

  std::vector<Vec2f> data;

  auto flow = (Xyz([](auto& p) { return p; }) + Stroke(Color::red()) +
               StrokeWeight(2) + Fill(Color::blue()) + PointSize(10)) >>
              scale >> scatter >> line;

  PerspectiveCamera camera(canvas.getRegion());
  canvas.bindOnResize([&camera, &scale](auto& canvas, auto width, auto height) {
    camera.setRegion(canvas.getRegion());
    scale->screenSpace = canvas.getRegion();
  });
  auto random = [] {
    using rand_t = decltype(rand());
    using limits_t = std::numeric_limits<rand_t>;
    return (rand() + limits_t::min()) /
           (limits_t::max() - (float)limits_t::lowest());
  };

  for (int i = 0; i < 10000; ++i) {
    data.emplace_back(random(), random());
  }

  flow.Apply(data.begin(), data.end());

  canvas.runForever([&](auto&&) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    root.render(camera);
    data.clear();
    for (int i = 0; i < 10000; ++i) {
      data.emplace_back(random(), random());
    }
    flow.Apply(data.begin(), data.end());
  });

  return 0;
}
