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

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::plot;
  using namespace emp::plot::properties;

  GLCanvas canvas(1000, 1000);

  auto g = graph(map(
    [](auto&& props) {
      auto& value = Value::get(props);

      return std::forward<decltype(props)>(props)
        .template set<X>(value.x())
        .template set<Y>(value.y())
        .template set<Fill>(Vec4f{1, 1, 1, 1});
    },
    all(Scatter(canvas), Line(canvas))));

  std::vector<Vec2f> data;
  for (int i = 0; i < 100; ++i) {
    data.emplace_back(i * 10 - 50, 200 * (rand() / (float)RAND_MAX) - 100);
  }

  auto proj =
    proj::orthoFromScreen(1000, 1000, canvas.getWidth(), canvas.getHeight());
  auto view = Mat4x4f::translation(0, 0, 0);

  canvas.runForever([&](auto&&) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    g.show(proj, view, data.begin(), data.end());
  });

  return 0;
}
