/*
 / This test compares the pngs generated in Canvas.cpp
 / to avoid a weird order-based SIGABRT that occured
 / while doing the comparisons in the same file.
*/
#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"


#include "emp/web/Canvas.hpp"

#ifdef EMP_HAS_SFML


TEST_CASE("Test Images MSE", "[native]") {
  std::string command = "python3 compare_images.py -f assets/circle_location.png -s test_circle_location.png";
  int success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/circle_location2.png -s test_circle_location2.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/circle_color.png -s test_circle_color.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/circle_size.png -s test_circle_size.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/circle_thickness.png -s test_circle_thickness.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/rect_location.png -s test_rect_location.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/rect_location.png -s test_rect_location2.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/rect_color.png -s test_rect_color.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/rect_size.png -s test_rect_size.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/line_location.png -s test_line_location.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/line_location.png -s test_line_location2.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/text_location.png -s test_text_location.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/text_location.png -s test_text_location2.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/text_color.png -s test_text_color.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/centertext_location.png -s test_centertext_location.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

  command = "python3 compare_images.py -f assets/centertext_location.png -s test_centertext_location2.png";
  success = std::system( command.c_str() );
  REQUIRE(success == 0);

}

#endif
