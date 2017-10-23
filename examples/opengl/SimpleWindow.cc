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
    attribute vec2 position;
    attribute vec4 color;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 proj;

    varying vec4 fcolor;

    void main()
    {
        gl_Position = proj * view * model * vec4(position, 0.0, 1.0);
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

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas;

  auto q = emp::math::Quat<float>::rotation(0, 0, 1, 5);

  auto shaderProgram = canvas.makeShaderProgram(vertexSource, fragmentSource);
  shaderProgram.use();

  gl::VertexArrayObject vao =
    canvas.makeVAO()
      .with(gl::BufferType::Array,
            shaderProgram.attribute<float[2]>("position", false,
                                              6 * sizeof(float), (void*)0),
            shaderProgram.attribute<float[4]>("color", false, 6 * sizeof(float),
                                              (void*)(2 * sizeof(float))))
      .with(gl::BufferType::ElementArray);

  vao.bind();
  vao.getBuffer<gl::BufferType::Array>().push(
    {
      -50.5f,  100.5f,  1.0f, 1.0f, 1.0f, 1.0f,  // Vertex 1 (X, Y)
      100.5f,  100.5f,  1.0f, 0.0f, 1.0f, 1.0f,  // Vertex 2 (X, Y)
      100.5f,  -100.5f, 1.0f, 1.0f, 0.0f, 1.0f,  // Vertex 3 (X, Y)
      -100.5f, -100.5f, 1.0f, 1.0f, 1.0f, 1.0f,  // Vertex 3 (X, Y)
    },
    gl::BufferUsage::StaticDraw);

  auto modelId = glGetUniformLocation(shaderProgram, "model");
  constexpr auto model = Mat4x4<float>::identity();
  glUniformMatrix4fv(modelId, 1, GL_FALSE, model.data());
  auto viewId = glGetUniformLocation(shaderProgram, "view");
  constexpr auto view = Mat4x4<float>::identity();
  glUniformMatrix4fv(viewId, 1, GL_FALSE, view.data());
  auto projId = glGetUniformLocation(shaderProgram, "proj");
  auto proj = proj::ortho(-200, 200, -200, 200, 0, 1).transpose();
  std::cout << proj << std::endl;
  glUniformMatrix4fv(projId, 1, GL_FALSE, proj.data());

  vao.getBuffer<gl::BufferType::ElementArray>().push(
    {
      0, 1, 2,  // First Triangle
      2, 3, 0   // Second Triangle
    },
    gl::BufferUsage::StaticDraw);

  canvas.runForever([](auto&&) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  });

  return 0;
}
