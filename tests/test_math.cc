#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "math/LinAlg.h"

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
