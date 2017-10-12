#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "math/LinAlg.h"

constexpr std::size_t rowsCheckGenerator(std::size_t r, std::size_t) {
  return r;
}

TEST_CASE("Test Matrices", "[math]") {
  constexpr auto rowsCheck =
    emp::math::MatUtils<3, 3>::generate(&rowsCheckGenerator);

  REQUIRE((rowsCheck == emp::math::Mat3x3<std::size_t>{
                          0u, 0u, 0u,  // row 1
                          1u, 1u, 1u,  // row 2
                          2u, 2u, 2u   // row 3
                        }));

  constexpr auto ident = emp::math::MatUtils<3, 3>::identity<float>;
  auto identManual = emp::math::Mat3x3<float>{
    1.0f, 0.0f, 0.0f,  // row 1
    0.0f, 1.0f, 0.0f,  // row 2
    0.0f, 0.0f, 1.0f   // row 3
  };

  REQUIRE(ident == identManual);

  constexpr auto rowVec = emp::math::Mat<int, 1, 3>{1, 2, 3};
  constexpr auto colVec = emp::math::Mat<int, 3, 1>{1, 2, 3};
  REQUIRE(rowVec(0, 0) == 1);
  REQUIRE(rowVec(0, 1) == 2);
  REQUIRE(rowVec * colVec == 1 + 2 * 2 + 3 * 3);
  REQUIRE(colVec * ident[0] == 1);
  REQUIRE(colVec * ident[1] == 2);
  REQUIRE(colVec * ident[2] == 3);
}
