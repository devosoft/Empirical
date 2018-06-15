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
// #include "plot/scales.h"
// #include "plot/scatter.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"
// #include "scenegraph/shapes.h"
#include "plot/flow.h"
#include "scenegraph/shapes.h"
#include "scenegraph/transform.h"

#include "scenegraph/rendering.h"

#include <chrono>
#include <cstdlib>

struct Particle {
  float mass;
  emp::math::Vec3f position;
  emp::math::Vec3f velocity;
  emp::math::Vec3f acceleration;

  Particle(float mass, const emp::math::Vec3f& position)
    : mass(mass), position(position) {}

  void Step(float dt) {
    velocity += acceleration * dt;
    position += velocity * dt;
    acceleration = {0, 0, 0};
  }

  void AddForce(const emp::math::Vec3f& force) { acceleration += force / mass; }
};

emp::scenegraph::FreeType ft;

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::math;
  using namespace emp::scenegraph;
  using namespace emp::graphics;
  // using namespace emp::plot;
  // using namespace emp::plot::attributes;

  using namespace emp::scenegraph::shapes;

  GLCanvas canvas;
  shaders::LoadShaders(canvas);

  emp::Resources<FontFace>::Add("Roboto", [] {
    auto font = ft.load("Assets/RobotoMono-Regular.ttf");
    font.SetPixelSize(0, 64);
    font.BulidAsciiAtlas();
    return font;
  });

  Region3f region = SetAspectRatioMax(Region2f{{-100, -100}, {100, 100}},
                                      AspectRatio(canvas.getRegion()))
                      .AddDimension(-100, 100);

  Stage stage(region);
  // auto root = stage.MakeRoot<Group>();
  // auto line{std::make_shared<Line>(canvas)};
  // auto scatter{std::make_shared<Scatter>(canvas, 6)};
  // auto scale{std::make_shared<Scale<3>>(region)};

  // auto r = std::make_shared<FilledRectangle>(canvas, Region2f{{0, 0}, {8,
  // 8}}); root->AttachAll(r, scatter);

  std::vector<Particle> particles;

  // auto flow = (Xyz([](auto& p) { return p.AddRow(0); }) +
  // Stroke(Color::red()) +
  //              StrokeWeight(2) + Fill(Color::blue()) + PointSize(10)) >>
  //             scale >> scatter /*>> line*/;

  // auto camera = std::make_shared<PerspectiveCamera>(
  //   consts::pi<float> / 4, canvas.getWidth() / (float)canvas.getHeight(),
  //   0.1, 100);

  auto camera = std::make_shared<OrthoCamera>(region);
  auto eye = std::make_shared<SimpleEye>();
  // eye.LookAt({40, 30, 30}, {0, 0, 0}, {0, 0, -1});

  // canvas.on_resize_event.bind(
  //   [&camera, &scale](auto& canvas, auto width, auto height) {
  //     // camera.setRegion(canvas.getRegion());
  //     // scale->screenSpace = canvas.getRegion();
  //     std::cout << width << " x " << height << std::endl;
  //   });

  for (int i = 0; i < 100000; ++i) {
    particles.emplace_back(10, Vec3f{rand() % 100 - 50, rand() % 100 - 50, 0});
  }

  // flow.Apply(data.begin(), data.end());

  float flength = 0;
  float clength;
  int count = 0;

  auto lastFrame = std::chrono::high_resolution_clock::now();

  emp::graphics::Graphics g(canvas, "Roboto", camera, eye);
  float t = 0;
  canvas.runForever([&](auto&&) {
    auto start = std::chrono::high_resolution_clock::now();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto start_compute = std::chrono::high_resolution_clock::now();
    for (auto& p : particles) {
      p.Step(0.1);
    }

    for (int i = 0; i < particles.size(); ++i) {
      auto& p1 = particles[i];
      p1.AddForce(
        {10 * (rand() / (float)std::numeric_limits<decltype(rand())>::max()) -
           5,
         10 * (rand() / (float)std::numeric_limits<decltype(rand())>::max()) -
           5,
         0});
    }

    for (auto& p : particles) {
      if (p.position.x() < region.min.x() || p.position.x() > region.max.x())
        p.velocity.x() *= -1;
      if (p.position.y() < region.min.y() || p.position.y() > region.max.y())
        p.velocity.y() *= -1;
    }
    auto finished_compute = std::chrono::high_resolution_clock::now();

    auto pen = g.FillRegularPolygons(32, {5, 5});
    for (int i = 0; i < particles.size(); ++i) {
      pen.Draw({
        emp::graphics::Fill = Color::red(1, 0.5),
        emp::graphics::Transform = Mat4x4f::Translation(particles[i].position),
      });
    }
    pen.Flush();
    t += 0.1;

    // stage.Render(camera, eye);
    // data.clear();
    // for (int i = 0; i < 10000; ++i) {
    //   data.emplace_back(random(), random());
    // }
    // flow.Apply(data.begin(), data.end());

    flength +=
      std::chrono::duration_cast<std::chrono::milliseconds>(start - lastFrame)
        .count();
    clength += std::chrono::duration_cast<std::chrono::microseconds>(
                 finished_compute - start_compute)
                 .count();
    lastFrame = start;

    if (count++ == 100) {
      auto mean_flength = flength / 100.0;
      auto mean_fps = 1000.0 / mean_flength;
      auto mean_clength = clength / 100.0;

      std::cout << particles.size()
                << " particles (last 100 frames):" << std::endl
                << "\tFPS          = " << mean_fps << std::endl
                << "\tFrame Length = " << mean_flength << " milliseconds"
                << std::endl
                << "\tCompute Time = " << mean_clength << " microseconds"
                << std::endl;
      clength = flength = count = 0;
    }
  });

  return 0;
}
