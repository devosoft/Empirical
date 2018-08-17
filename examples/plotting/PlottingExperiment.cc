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
#include "opengl/defaultShaders.h"
#include "opengl/glcanvas.h"
// #include "plot/line.h"
#include "plot/scales.h"
#include "plot/scatter.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"
// #include "scenegraph/shapes.h"
#include "plot/flow.h"
#include "scenegraph/transform.h"

#include "scenegraph/rendering.h"

#include <chrono>
#include <cstdlib>

struct Particle {
  float mass;
  emp::math::Vec2f position;
  emp::math::Vec2f velocity;
  emp::math::Vec2f acceleration;

  Particle(float mass, const emp::math::Vec2f& position)
    : mass(mass), position(position) {}

  void Step(float dt) {
    velocity += acceleration * dt;
    position += velocity * dt;
    acceleration = {0, 0};
  }

  void AddForce(const emp::math::Vec2f& force) { acceleration += force / mass; }
};

template <typename P, typename R>
void UpdateParticles(P& particles, const R& region) {
  for (auto& p : particles) {
    p.Step(0.1);
  }

  for (int i = 0; i < particles.size(); ++i) {
    auto& p1 = particles[i];
    p1.AddForce({
      10 * (rand() / (float)std::numeric_limits<decltype(rand())>::max()) - 5,
      10 * (rand() / (float)std::numeric_limits<decltype(rand())>::max()) - 5,
    });
  }

  // for (auto& p : particles) {
  //   if (p.position.x() < region.min.x() || p.position.x() > region.max.x())
  //     p.velocity.x() *= -1;
  //   if (p.position.y() < region.min.y() || p.position.y() > region.max.y())
  //     p.velocity.y() *= -1;
  // }
}

emp::scenegraph::FreeType ft;

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::math;
  using namespace emp::scenegraph;
  using namespace emp::graphics;
  using namespace emp::plot;
  using namespace emp::plot::attributes;

  GLCanvas canvas(500, 500);
  shaders::LoadShaders(canvas);

  emp::Resources<FontFace>::Add("Roboto", [] {
    auto font = ft.load("Assets/RobotoMono-Regular.ttf");
    font.SetFreeTypePixelSize(0, 64);
    font.BulidAsciiAtlas();
    return font;
  });

  Stage<2> stage(canvas.getRegion());
  auto root = stage.MakeRoot<Flow<2>>(true, FlowDirection<2>::Y);
  // auto line{std::make_shared<Line>(canvas)};
  auto scatter{std::make_shared<Scatter<2>>(Mesh::Polygon(32, {2, 2}))};
  auto scale{std::make_shared<Scale<2>>()};

  auto plot{std::make_shared<Stack<2>>()};
  auto plot_title{std::make_shared<Text<2>>("Hello World", 32)};
  plot->Append(scatter).Append(scale);
  root->Append(plot_title, 0).Append(plot);

  std::vector<Particle> particles;

  auto flow = MakeFlow().Then(scale).Then(scatter).Data(MakeAttrs(
    Xyz = [](auto& p) { return p.position; }, PointSize = 1,
    emp::graphics::Fill = Color::red(1, 0.5), emp::graphics::TextSize = 16));

  auto camera =
    std::make_shared<OrthoCamera>(canvas.getRegion().AddDimension(-100, 100));
  auto eye = std::make_shared<SimpleEye>();

  for (int i = 0; i < 10000; ++i) {
    particles.emplace_back(10, Vec2f{rand() % 100 - 50, rand() % 100 - 50});
  }

  emp::graphics::Graphics g(canvas, "Roboto", camera, eye);
  canvas.runForever([&](auto&&) {
    g.Clear(Color::grey(0.8));

    UpdateParticles(particles, canvas.getRegion());
    flow(particles.begin(), particles.end());

    stage.Render(g);
  });

  return 0;
}
