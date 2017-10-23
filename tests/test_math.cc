#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "math/LinAlg.h"
#include "math/consts.h"

constexpr std::size_t rowsCheckGenerator(std::size_t r, std::size_t) {
  return r + 1;
}

using namespace emp::math;

#define CONST_REQUIRE_EQ(A, B) \
  {                            \
    constexpr auto __a = A;    \
    constexpr auto __b = B;    \
    REQUIRE(__a == __b);       \
  }
#define CONST_REQUIRE(X)    \
  {                         \
    constexpr auto __x = X; \
    REQUIRE(__x);           \
  }

TEST_CASE("Test Matrices", "[math]") {
  constexpr auto rowsCheck = Mat3x3s::from(&rowsCheckGenerator);

  CONST_REQUIRE_EQ(rowsCheck, (Mat3x3s{
                                1u, 1u, 1u,  // row 1
                                2u, 2u, 2u,  // row 2
                                3u, 3u, 3u   // row 3
                              }));

  constexpr auto ident = Mat3x3f::identity();

  CONST_REQUIRE_EQ(ident, (Mat3x3f{
                            1.0f, 0.0f, 0.0f,  // row 1
                            0.0f, 1.0f, 0.0f,  // row 2
                            0.0f, 0.0f, 1.0f   // row 3
                          }));

  constexpr auto rowVec = RowVec3i{1, 2, 3};
  constexpr auto colVec = ColVec3i{1, 2, 3};
  CONST_REQUIRE_EQ(rowVec(0, 0), 1);
  CONST_REQUIRE_EQ(rowVec(0, 1), 2);
  CONST_REQUIRE_EQ(rowVec * colVec, 1 + 2 * 2 + 3 * 3);
  CONST_REQUIRE_EQ(ident.row(0) * ident.col(0), 1);
  CONST_REQUIRE_EQ(ident.row(1) * ident.col(1), 1);
  CONST_REQUIRE_EQ(ident.row(2) * ident.col(2), 1);

  constexpr auto colVecId = rowVec * ident;
  CONST_REQUIRE_EQ(colVecId(0, 0), 1);
  CONST_REQUIRE_EQ(colVecId(0, 1), 2);
  CONST_REQUIRE_EQ(colVecId(0, 2), 3);

  CONST_REQUIRE_EQ(ident * rowsCheck, rowsCheck);
  CONST_REQUIRE_EQ(rowsCheck * ident, rowsCheck);

  CONST_REQUIRE_EQ(rowsCheck * rowsCheck, (Mat3x3s{
                                            6u, 6u, 6u,     // row 1
                                            12u, 12u, 12u,  // row 2
                                            18u, 18u, 18u   // row 3
                                          }));
  CONST_REQUIRE_EQ(rowsCheck.transpose(), (Mat3x3s{
                                            1u, 2u, 3u,  // row 1
                                            1u, 2u, 3u,  // row 2
                                            1u, 2u, 3u   // row 3
                                          }));

  CONST_REQUIRE(cross({1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}).feq({-3.f, 6.f, -3.f}));
  CONST_REQUIRE_EQ((Vec3i{1, 2, 3}).magSq(), 14);
  // TODO: implement constexpr sqrt
  // CONST_REQUIRE_EQ((Vec2i{3, 4}).mag(), 5);
  REQUIRE((Vec2i{3, 4}).mag() == 5);
}

TEST_CASE("Test Quaternions", "[math]") {
  constexpr emp::math::Quat<int> a{1, 2, 3, 4};
  CONST_REQUIRE_EQ(a * a, Quat<int>(-28, 4, 6, 8));
  CONST_REQUIRE_EQ((a * 0.5), Quat<double>(0.5, 1, 1.5, 2));

  emp::math::Quat<int> b{1, 2, 3, 4};
  REQUIRE((b *= a) == Quat<int>(-28, 4, 6, 8));
  REQUIRE((b *= 2) == Quat<double>(-56, 8, 12, 16));
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

  constexpr auto rot = Mat4x4f{
    1.f, 0.f, 0.f,  0.f,  // row 1
    0.f, 0.f, -1.f,
    0.f,                  // row 2
    0.f, 1.f, 0.f,  0.f,  // row 3
    0.f, 0.f, 0.f,  1.f,  // row 4
  };
  REQUIRE(
    (Quat<float>::rotation(consts::pi<float> / 2, {1.f, 0.f, 0.f}).rotMat())
      .feq(rot));
  auto r = RowVec4f{1.f, 0.f, 0.f, 1.f};
  REQUIRE((r * (Quat<float>::rotation(consts::pi<float> / 2, {1.f, 0.f, 0.f})
                  .rotMat())) == r);
}
