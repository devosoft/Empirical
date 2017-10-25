//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//

#include <iostream>

#include "Evo/NK.h"
#include "Evo/World.h"
#include "config/ArgManager.h"
#include "tools/BitVector.h"
#include "tools/Random.h"

#include "math/LinAlg.h"
#include "opengl/defaultShaders.h"
#include "opengl/glcanvas.h"

EMP_BUILD_CONFIG(
  NKConfig, GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, uint32_t, 10, "Level of epistasis in the NK model"),
  VALUE(N, uint32_t, 20, "Number of bits in each organisms (must be > K)"),
  ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time) "),
  ALIAS(NUM_MUTS),
  VALUE(TEST, std::string, "TestString", "This is a test string."))

using BitOrg = emp::BitVector;

namespace gl = emp::opengl;
using namespace emp::math;

template <typename V, typename M>
V wrap(V v, M&& max) {
  v %= max;
  if (v < 0) v += std::forward<M>(max);
  return v;
}

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas;
  emp::Random random;

  auto q = emp::math::Quat<float>::rotation(0, 0, 1, 5);

  auto shaderProgram = gl::shaders::simpleSolidColor(canvas);
  shaderProgram.use();

  gl::VertexArrayObject vao =
    canvas.makeVAO()
      .with(gl::BufferType::Array, shaderProgram.attribute<Vec3f>("position"))
      .with(gl::BufferType::ElementArray);

  constexpr auto SIZE = 20;
  vao.bind();
  vao.getBuffer<gl::BufferType::Array>().set(
    {
      Vec3f{-SIZE / 2, SIZE / 2, 0},
      Vec3f{SIZE / 2, SIZE / 2, 0},
      Vec3f{SIZE / 2, -SIZE / 2, 0},
      Vec3f{-SIZE / 2, -SIZE / 2, 0},
    },
    gl::BufferUsage::StaticDraw);

  auto color = shaderProgram.uniform("color");

  auto model = shaderProgram.uniform("model");
  auto view = shaderProgram.uniform("view");
  view.set(Mat4x4f::identity());
  auto proj = shaderProgram.uniform("proj");

  constexpr auto WIDTH = 500;
  constexpr auto HEIGHT = 500;

  proj.set(proj::ortho(WIDTH * SIZE, HEIGHT * SIZE,
                       canvas.getWidth() / (float)canvas.getHeight()));

  vao.getBuffer<gl::BufferType::ElementArray>().set(
    {
      0, 1, 2,  // First Triangle
      2, 3, 0   // Second Triangle
    },
    gl::BufferUsage::StaticDraw);

  std::vector<std::vector<bool>> current(WIDTH, std::vector<bool>(HEIGHT));
  std::vector<std::vector<bool>> next(WIDTH, std::vector<bool>(HEIGHT));

  for (int x = 0; x < WIDTH; ++x) {
    for (int y = 0; y < HEIGHT; ++y) {
      current[x][y] = random.P(0.5);
    }
  }

  canvas.runForever([&](auto&&) {

    for (int x = 0; x < WIDTH; ++x) {
      for (int y = 0; y < HEIGHT; ++y) {
        auto n = current[wrap(x - 1, WIDTH)][wrap(y + 1, HEIGHT)] +
                 current[x][wrap(y + 1, HEIGHT)] +
                 current[wrap(x + 1, WIDTH)][wrap(y + 1, HEIGHT)] +  // TOP
                 current[wrap(x - 1, WIDTH)][y] +
                 current[wrap(x + 1, WIDTH)][y] +  // CENTER
                 current[wrap(x - 1, WIDTH)][wrap(y - 1, HEIGHT)] +
                 current[x][wrap(y - 1, HEIGHT)] +
                 current[wrap(x + 1, WIDTH)][wrap(y - 1, HEIGHT)];  // BOTTOM
        if (current[x][y]) {
          next[x][y] = n == 2 || n == 3;
        } else if (!current[x][y] && n == 3)
          next[x][y] = true;
      }
    }
    current = next;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int x = 0; x < WIDTH; ++x) {
      for (int y = 0; y < HEIGHT; ++y) {
        if (current[x][y]) {
          color.set(Vec4f{.75, .25, .75, 1});
        } else {
          color.set(Vec4f{.75, .75, .25, 1});
        }
        model.set(Mat4x4f::translation((x - WIDTH / 2) * SIZE + SIZE / 2,
                                       (y - HEIGHT / 2) * SIZE + SIZE / 2));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
    }
  });

  return 0;
}
