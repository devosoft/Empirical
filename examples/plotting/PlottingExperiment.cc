//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//

#include <iostream>
#include <limits>

#include "math/LinAlg.h"
#include "math/consts.h"
#include "opengl/defaultShaders.h"
#include "opengl/glcanvas.h"
namespace gl = emp::opengl;
using namespace emp::math;

template <typename T>
class Scatter {
  private:
  gl::shaders::SimpleSolidColor shader;

  std::function<float(const T&)> x;
  std::function<float(const T&)> y;
  std::function<Vec4f(const T&)> color;
  std::function<float(const T&)> weight;

  public:
  Scatter(gl::GLCanvas& canvas, std::function<float(const T&)> x,
          std::function<float(const T&)> y,
          std::function<Vec4f(const T&)> color,
          std::function<float(const T&)> weight)
    : shader(canvas), x(x), y(y), color(color), weight(weight) {
    shader.shader.use();
    shader.vao.getBuffer<gl::BufferType::Array>().set(
      {
        Vec3f{-0.5, +0.5, 0},
        Vec3f{+0.5, +0.5, 0},
        Vec3f{+0.5, -0.5, 0},
        Vec3f{-0.5, -0.5, 0},
      },
      gl::BufferUsage::StaticDraw);
    shader.vao.getBuffer<gl::BufferType::ElementArray>().set(
      {
        0, 1, 2,  // First Triangle
        2, 3, 0   // Second Triangle
      },
      gl::BufferUsage::StaticDraw);
  }

  template <typename P, typename V, typename Iter>
  void show(P&& proj, V&& view, Iter begin, Iter end) {
    shader.shader.use();
    shader.proj.set(std::forward<P>(proj));
    shader.view.set(std::forward<V>(view));
    for (auto iter = begin; iter != end; ++iter) {
      shader.model.set(Mat4x4f::translation(x(*iter), y(*iter)));
      shader.color.set(color(*iter));
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
  }
};

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas(1000, 1000);

  auto scatter = Scatter<Vec2f>(canvas, [](auto& v) { return v.x(); },
                                [](auto& v) { return v.y(); },
                                [](auto&) {
                                  return Vec4f{1.0, 1.0, 1.0, 1.0};
                                },
                                [](auto&) { return 1.0; });

  std::vector<Vec2f> data{
    Vec2f{0.1, 0.1}, Vec2f{1, 1},     Vec2f{2, 2},     Vec2f{4, 4},
    Vec2f{8, 8},     Vec2f{16, 16},   Vec2f{32, 32},   Vec2f{64, 64},
    Vec2f{128, 128}, Vec2f{256, 256}, Vec2f{512, 512}, Vec2f{1024, 1024},
  };
  auto proj =
    proj::orthoFromScreen(1000, 1000, canvas.getWidth(), canvas.getHeight());
  auto view = Mat4x4f::translation(0, 0, 0);

  canvas.runForever([&](auto&&) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    scatter.show(proj, view, data.begin(), data.end());
  });

  return 0;
}
