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
#include "opengl/texture.h"
#include "scenegraph/camera.h"
#include "scenegraph/core.h"
#include "scenegraph/freetype.h"
#include "scenegraph/shapes.h"
#include "scenegraph/transform.h"

#include <chrono>
#include <cstdlib>

emp::scenegraph::FreeType ft;

int main(int argc, char* argv[]) {
  using namespace emp::opengl;
  using namespace emp::math;
  using namespace emp::scenegraph;
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

  constexpr auto width = 64;
  constexpr auto height = 64;
  std::vector<uint8_t> pixels(width * height * 4);
  for (int i = pixels.size() - 1; i >= 0; --i) {
    pixels[i] = 255 / (i % 20 + 1);
  }

  auto texture = std::make_shared<Texture2d>();
  texture->Data(Texture2DFormat::RGBA, width, height, pixels);
  texture->SetMinFilter(TextureMinFilter::Linear);
  texture->SetMagFilter(TextureMagFilter::Linear);

  root->Attach(
    std::make_shared<TextureView>(canvas, Region2f{{0, 0}, {8, 8}}, texture));

  root->Attach(std::make_shared<Text>(canvas, "Hello World", "Roboto"));

  OrthoCamera camera(region);

  // PerspectiveCamera camera(consts::pi<float> / 4,
  //                          canvas.getWidth() / (float)canvas.getHeight(),
  //                          0.1, 1000);

  SimpleEye eye;
  // OrbitController orbit({40, 30, 30});
  // eye.LookAt({10, 10, 10}, {0, 1, 0}, {0, 0, 0});

  canvas.runForever([&](auto&&) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // orbit.Apply(eye);
    stage.Render(camera, eye);
  });

  // glDeleteTextures(1, &name);

  return 0;
}
