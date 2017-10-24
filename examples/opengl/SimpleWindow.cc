//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h with an NK
//  landscape.

#include <iostream>

#include "Evo/NK.h"
#include "Evo/World.h"
#include "config/ArgManager.h"
#include "tools/BitVector.h"
#include "tools/Random.h"

#include "math/LinAlg.h"
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

const char* vertexSource = R"glsl(
    attribute vec3 position;
    attribute vec4 color;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 proj;

    varying vec4 fcolor;

    void main()
    {
        gl_Position = proj * view * model * vec4(position, 1.0);
        fcolor = color;
    }
)glsl";

const char* fragmentSource = R"glsl(
    varying vec4 fcolor;

    void main()
    {
        gl_FragColor = fcolor;
    }
)glsl";

namespace gl = emp::opengl;
using namespace emp::math;

struct Vert {
  Vec3f position;
  float color[4];
};

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas;

  auto q = emp::math::Quat<float>::rotation(0, 0, 1, 5);

  auto shaderProgram = canvas.makeShaderProgram(vertexSource, fragmentSource);
  shaderProgram.use();

  gl::VertexArrayObject vao =
    canvas.makeVAO()
      .with(gl::BufferType::Array,
            shaderProgram.attribute("position", &Vert::position),
            shaderProgram.attribute("color", &Vert::color))
      .with(gl::BufferType::ElementArray);

  vao.bind();
  vao.getBuffer<gl::BufferType::Array>().set(
    {
      Vert{{-100, 100, 0}, {1.0f, 1.0f, 1.0f, 1.0f}},   // Vertex 1 (X, Y)
      Vert{{100, 100, 0}, {1.0f, 0.0f, 1.0f, 1.0f}},    // Vertex 2 (X, Y)
      Vert{{100, -100, 0}, {1.0f, 1.0f, 0.0f, 1.0f}},   // Vertex 3 (X, Y)
      Vert{{-100, -100, 0}, {1.0f, 1.0f, 1.0f, 1.0f}},  // Vertex 3 (X, Y)
    },
    gl::BufferUsage::StaticDraw);

  shaderProgram.uniform("model").set(Mat4x4f::translation(0, 10));
  shaderProgram.uniform("view").set(Mat4x4f::identity());
  shaderProgram.uniform("proj").set(proj::ortho(-200, 200, -200, 200, 0, 1));

  vao.getBuffer<gl::BufferType::ElementArray>().set(
    {
      0, 1, 2,  // First Triangle
      2, 3, 0   // Second Triangle
    },
    gl::BufferUsage::StaticDraw);

  canvas.runForever([&](auto&&) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  });

  return 0;
}
