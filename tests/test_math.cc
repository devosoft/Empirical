#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "math/LinAlg.h"
#include "math/consts.h"

constexpr std::size_t rowsCheckGenerator(std::size_t r, std::size_t) {
  return r + 1;
}

using namespace emp::math;

TEST_CASE("Test Matrices", "[math]") {
  constexpr auto rowsCheck = MatUtils<3, 3>::generate(&rowsCheckGenerator);

  REQUIRE((rowsCheck == Mat3x3<std::size_t>{
                          1u, 1u, 1u,  // row 1
                          2u, 2u, 2u,  // row 2
                          3u, 3u, 3u   // row 3
                        }));

  constexpr auto ident = MatUtils<3, 3>::identity<float>;
  auto identManual = Mat3x3<float>{
    1.0f, 0.0f, 0.0f,  // row 1
    0.0f, 1.0f, 0.0f,  // row 2
    0.0f, 0.0f, 1.0f   // row 3
  };

  REQUIRE(ident == identManual);

  constexpr auto rowVec = Mat<int, 1, 3>{1, 2, 3};
  constexpr auto colVec = Mat<int, 3, 1>{1, 2, 3};
  REQUIRE(rowVec(0, 0) == 1);
  REQUIRE(rowVec(0, 1) == 2);
  REQUIRE(rowVec * colVec == 1 + 2 * 2 + 3 * 3);
  REQUIRE(ident.row(0) * ident.col(0) == 1);
  REQUIRE(ident.row(1) * ident.col(1) == 1);
  REQUIRE(ident.row(2) * ident.col(2) == 1);

  REQUIRE(colVec * ident.row(0) == 1);
  REQUIRE(colVec * ident.row(1) == 2);
  REQUIRE(colVec * ident.row(2) == 3);

  REQUIRE(ident * rowsCheck == rowsCheck);
  REQUIRE(rowsCheck * ident == rowsCheck);

  REQUIRE((rowsCheck * rowsCheck == Mat3x3<std::size_t>{
                                      6u, 6u, 6u,     // row 1
                                      12u, 12u, 12u,  // row 2
                                      18u, 18u, 18u   // row 3
                                    }));
  REQUIRE((rowsCheck.transpose() == Mat3x3<std::size_t>{
                                      1u, 2u, 3u,  // row 1
                                      1u, 2u, 3u,  // row 2
                                      1u, 2u, 3u   // row 3
                                    }));
}

TEST_CASE("Test Quaternions", "[math]") {
  emp::math::Quat<int> a{1, 2, 3, 4};

  REQUIRE(a * a == Quat<int>(-28, 4, 6, 8));
  REQUIRE((a *= a) == Quat<int>(-28, 4, 6, 8));
  REQUIRE((a * 0.5) == Quat<double>(-14, 2, 3, 4));
  REQUIRE((a *= 2) == Quat<double>(-56, 8, 12, 16));
  REQUIRE(Quat<float>::rotation(1.f, 1.f, 0.f, 0.f)
            .feq({0.8775826, 0.4794255, 0, 0}));

  REQUIRE(Quat<float>::rotation(consts::pi<float>, {1.f, 0.f, 0.f})
            .feq({0, 1, 0, 0}));
  REQUIRE(Quat<float>::rotation(2 * consts::pi<float>, {1.f, 0.f, 0.f})
            .feq({-1, 0, 0, 0}));
  REQUIRE(Quat<float>::rotation(consts::pi<float>, {0.f, 1.f, 0.f})
            .feq({0, 0, 1, 0}));

  REQUIRE(Quat<float>::rotation(consts::pi<float> / 2, {1.f, 1.f, 0.f})
            .feq({0.7071068f, 0.5f, 0.5f, 0}));

  // std::cout << Quat<float>::rotation(consts::pi<float> / 2,
  //                                    Mat<float, 3, 1>{1.f, 0.f, 0.f})
  //                .rotMat()
  //           << std::endl
  //           << std::endl;

  // std::cout << Mat4x4<float>{
  //                          1.f, 0.f, 0.f, 0.f,  // row 1
  //                          0.f, 0.f, -1.f,
  //                          0.f,                 // row 2
  //                          0.f, 1.f, 0.f, 0.f,  // row 3
  //                          0.f, 0.f, 0.f, 1.f,  // row 4
  //                        } << std::endl;

  auto rot = Mat4x4<float>{
    1.f, 0.f, 0.f,  0.f,  // row 1
    0.f, 0.f, -1.f,
    0.f,                  // row 2
    0.f, 1.f, 0.f,  0.f,  // row 3
    0.f, 0.f, 0.f,  1.f,  // row 4
  };
  REQUIRE((Quat<float>::rotation(consts::pi<float> / 2,
                                 Mat<float, 3, 1>{1.f, 0.f, 0.f})
             .rotMat())
            .feq(rot));
  auto r = Mat<float, 1, 4>{1.f, 0.f, 0.f, 1.f};
  REQUIRE((r * (Quat<float>::rotation(consts::pi<float> / 2,
                                      Mat<float, 3, 1>{1.f, 0.f, 0.f})
                  .rotMat())) == r);
}
