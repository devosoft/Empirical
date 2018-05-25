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
#include "plot/line.h"
#include "plot/scales.h"
#include "plot/scatter.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "tools/attrs.h"
// #include "scenegraph/shapes.h"
#include "scenegraph/shapes.h"
#include "scenegraph/transform.h"

#include "scenegraph/rendering.h"

#include <chrono>
#include <cstdlib>

emp::scenegraph::FreeType ft;

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::math;
  using namespace emp::scenegraph;
  using namespace emp::plot;
  using namespace emp::plot::attributes;

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
  auto root = stage.MakeRoot<Group>();
  // auto line{std::make_shared<Line>(canvas)};
  auto scatter{std::make_shared<Scatter>(canvas, 6)};
  auto scale{std::make_shared<Scale<3>>(region)};

  auto r = std::make_shared<FilledRectangle>(canvas, Region2f{{0, 0}, {8, 8}});
  root->AttachAll(r, scatter);

  std::vector<Vec2f> data;

  // auto flow = (Xyz([](auto& p) { return p.AddRow(0); }) +
  // Stroke(Color::red()) +
  //              StrokeWeight(2) + Fill(Color::blue()) + PointSize(10)) >>
  //             scale >> scatter /*>> line*/;

  // PerspectiveCamera camera(consts::pi<float> / 4,
  //                          canvas.getWidth() / (float)canvas.getHeight(),
  //                          0.1, 100);

  OrthoCamera camera(region);
  SimpleEye eye;
  // eye.LookAt({40, 30, 30}, {0, 0, 0}, {0, 0, -1});

  // canvas.on_resize_event.bind(
  //   [&camera, &scale](auto& canvas, auto width, auto height) {
  //     // camera.setRegion(canvas.getRegion());
  //     // scale->screenSpace = canvas.getRegion();
  //     std::cout << width << " x " << height << std::endl;
  //   });

  for (int i = 0; i < 100; ++i) {
    data.emplace_back(i * 100, i * 100);
  }

  // flow.Apply(data.begin(), data.end());

  emp::graphics::Graphics g(canvas, "Roboto");

  canvas.runForever([&](auto&&) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    g.FillRegularPolygons({camera.GetProjection(), eye.CalculateView()}, 5,
                          {10, 10})
      .Draw({emp::graphics::Transform(Mat4x4f::Translation(5, 5, 0)),
             emp::graphics::Color(Color::red())})
      .Draw({
        emp::graphics::Color(Color::red()),
        emp::graphics::Transform(Mat4x4f::Translation(-5, -5, 0)),
      });

    g.Text({camera.GetProjection(), eye.CalculateView()})
      .Draw({
        emp::graphics::Transform(Mat4x4f::Translation(5, 5, 0)),
        emp::graphics::Color(Color::red()),
        emp::graphics::Text("Hello World"),
      });

    // stage.Render(camera, eye);
    // data.clear();
    // for (int i = 0; i < 10000; ++i) {
    //   data.emplace_back(random(), random());
    // }
    // flow.Apply(data.begin(), data.end());
  });

  return 0;
}
