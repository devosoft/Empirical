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
#include "plot/plot.h"

#include <cstdlib>

#include <chrono>

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::plot;
  using namespace emp::plot::properties;
  using namespace emp::math;

  constexpr auto MAX = 100;
  constexpr auto MIN = -100;
  constexpr auto STEPS = 1e3;
  constexpr auto f_STEPS = static_cast<float>(STEPS);

  GLCanvas canvas;
  canvas.bindOnMouseEvent(
    [](auto& canvas, auto& event) { std::cout << event << std::endl; });

  auto g{graph()
           .then_map(CartesianData::to(Value::get),
                     Fill::to(Vec4f{1, 1, 0.5, 1}),
                     Stroke::to(Vec4f{0.5, 0.5, 0.5, 1}), StrokeWeight::to(2),
                     PointSize::to(4))
           .then_views(canvas, Line(canvas), Scatter(canvas))};

  std::vector<Vec2f> data;
  for (int i = 0; i < STEPS; ++i) {
    float theta = (i / f_STEPS) * 2 * consts::pi<float>;
    data.emplace_back(sin(theta) * (MAX - MIN), cos(theta) * (MAX - MIN));
  }

  float offset = 0;
  canvas.runForever([&](auto&&) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    g(data.begin(), data.end(), canvas.getRegion());
    data.clear();
    for (int i = 0; i < STEPS; ++i) {
      float theta = (i / f_STEPS) * 2 * consts::pi<float> + offset;
      data.emplace_back((sin(theta) - cos(2 * theta)) / 3 * (MAX - MIN),
                        (sin(4 * theta) + cos(theta)) / 3 * (MAX - MIN));
    }
    offset += 0.005;
  });

  return 0;
}
