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

// float vertices[] = {
//     0.0f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f,  // Vertex 1 (X, Y)
//     0.5f,  -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,  // Vertex 2 (X, Y)
//     -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f   // Vertex 3 (X, Y)
// };

const char* vertexSource = R"glsl(
    attribute vec2 position;
    attribute vec4 color;

    varying vec4 fcolor;

    void main()
    {
        gl_Position = vec4(position, 0.0, 1.0);
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

// struct Test {
//   emp::web::Document doc;
//
//   Test() : doc("doc") {
//     std::cout << "Starting up..." << std::endl;
//     NKConfig config;
//     // config.Read("NK.cfg");
//
//     const uint32_t N = config.N();
//     const uint32_t K = config.K();
//
//     emp::Random random(config.SEED());
//     emp::NKLandscape landscape(N, K, random);
//
//     BitOrg start(N);
//     for (uint32_t j = 0; j < N; j++) start[j] = random.P(0.5);
//
//     auto fitnessLandscape = CreateFitnessLandscape<BitOrg>(
//       [N, &landscape](const BitOrg& root) {
//         std::vector<BitOrg> neighbors;
//         neighbors.reserve(N);
//
//         for (size_t i = 0; i < N; ++i) {
//           auto neighbor = root;
//           neighbor.Set(i, !root.Get(i));
//
//           neighbors.push_back(neighbor);
//         }
//         return neighbors;
//       },
//       [&](const BitOrg& org) { return landscape.GetFitness(org); });
//     std::cout << "Displaying fitnessLandscape..." << std::endl;
//
//     auto fitnessNeighborhood =
//     fitnessLandscape.GetFitnessNeighborhood(start); doc <<
//     SimpleDisplayForFitnessNeighborhood("fn", fitnessNeighborhood);
//   }
// };
//
// Test test;

namespace gl = emp::opengl;

int main(int argc, char* argv[]) {
  gl::GLCanvas canvas;

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
