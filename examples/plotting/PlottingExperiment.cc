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

#include <cstdlib>

namespace gl = emp::opengl;
using namespace emp::math;

template <typename F>
class Region2D {
  public:
  Vec2<F> min;
  Vec2<F> max;

  constexpr Region2D(F minX, F minY, F maxX, F maxY)
    : min{minX, minY}, max{maxX, maxY} {}
  constexpr Region2D(const Vec2<F>& min, const Vec2<F>& max)
    : min{min}, max{max} {}

  constexpr Region2D(const Region2D&) = default;
  constexpr Region2D(Region2D&&) = default;

  constexpr Region2D& operator=(const Region2D&) = default;
  constexpr Region2D& operator=(Region2D&&) = default;

  constexpr decltype(auto) width() const { return max.x - min.x; }
  constexpr decltype(auto) height() const { return max.y - min.y; }
  constexpr decltype(auto) size() const { return max - min; }

  Vec2<F> rescale(const Vec2<F>& value,
                  const Region2D<F>& originalScale) const {
    return {
      ((value.x - originalScale.min.x) / originalScale.width()) * width() +
        min.x,
      ((value.y - originalScale.min.y) / originalScale.height()) * height() +
        min.y,
    };
  }
};

template <typename T>
class Scatter {
  private:
  gl::shaders::SimpleSolidColor shader;

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

  template <typename P, typename Iter>
  void show(const P& parent, const Region2D<float>& dataRegion,
            const Region2D<float>& screen, Iter begin, Iter end) {
    shader.shader.use();
    shader.proj = std::forward<P>(parent.projection());
    shader.view = std::forward<V>(parent.view());

    // float dmaxX = std::numeric_limits<float>::lowest();
    // float dmaxY = std::numeric_limits<float>::lowest();
    //
    // float dminX = std::numeric_limits<float>::max();
    // float dminY = std::numeric_limits<float>::max();
    //
    // for (auto iter = begin; iter != end; ++iter) {
    //   float x_value{x(*iter)};
    //   float y_value{y(*iter)};
    //
    //   if (x_value > dmaxX) dmaxX = x_value;
    //   if (x_value < dminX) dminX = x_value;
    //
    //   if (y_value > dmaxY) dmaxY = y_value;
    //   if (y_value < dminY) dminY = y_value;
    // }

    for (auto iter = begin; iter != end; ++iter) {
      float px = ((x(*iter) - dminX) / (dmaxX - dminX)) * (maxX - minX) + minX;
      float py = ((y(*iter) - dminY) / (dmaxY - dminY)) * (maxY - minY) + minY;

      shader.model.set(Mat4x4f::translation(px, py));
      shader.color.set(color(*iter));

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
  }
};

template <typename T>
class Scales2d {
  public:
  std::function<float(const T&)> x;
  std::function<float(const T&)> y;

  private:
  template <typename X, typename Y>
  Scales2d(X&& x, Y&& y) : x(std::forward<X>(x)), y(std::forward<Y>(y)) {}

  template <typename P>
  void render(const P& parent) {}
};

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas(1000, 1000);

  auto scatter = Scatter<Vec2f>(canvas, [](auto& v) { return v.x(); },
                                [](auto& v) { return v.y(); },
                                [](auto&) {
                                  return Vec4f{1.0, 1.0, 1.0, 1.0};
                                },
                                [](auto&) { return 1.0; });

  constexpr auto SIZE{1e5};
  std::vector<Vec2f> data;
  data.reserve(SIZE);

  auto proj =
    proj::orthoFromScreen(1000, 1000, canvas.getWidth(), canvas.getHeight());
  auto view = Mat4x4f::translation(0, 0, 0);

  std::cout << "[[ STARTING RENDER ]]" << std::endl;
  canvas.runForever([&](auto&&) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    scatter.show(-500, -500, 500, 500, proj, view, data.begin(), data.end());
    if (data.size() <= SIZE) {
      for (std::size_t i = 0; i <= 10; ++i)
        data.emplace_back(std::rand() / (float)RAND_MAX,
                          std::rand() / (float)RAND_MAX);
    } else {
      std::cout << "DONE" << std::endl;
    }
  });

  return 0;
}
