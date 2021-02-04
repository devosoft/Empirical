#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"


#include "emp/web/_NativeCanvas.hpp"


TEST_CASE("Test Circle Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20);
  canvas.Circle(230, 40, 20);
  canvas.Circle(440, 40, 20);
  canvas.Circle(20, 440, 20);
  canvas.Circle(230, 440, 20);
  canvas.Circle(440, 440, 20);
  
  canvas.SavePNG("circle_location.png");

}

TEST_CASE("Test Circle Locations By Point", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.Circle(emp::Point(40,60), 20);
  canvas.Circle(emp::Point(250,60), 20);
  canvas.Circle(emp::Point(460,60), 20);
  canvas.Circle(emp::Point(40,460), 20);
  canvas.Circle(emp::Point(250,460), 20);
  canvas.Circle(emp::Point(460,460), 20);
  
  canvas.SavePNG("circle_location2.png");

}

TEST_CASE("Test Circle Colors", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20);
  canvas.Circle(80, 40, 20, "white", "red");
  canvas.Circle(140, 40, 20, "white", "yellow");
  canvas.Circle(200, 40, 20, "white", "blue");

  canvas.Circle(20, 120, 20, "black", "white");
  canvas.Circle(80, 120, 20, "red", "black");
  canvas.Circle(140, 120, 20, "yellow", "black");
  canvas.Circle(200, 120, 20, "blue", "black");
  
  canvas.SavePNG("circle_color.png");

}

TEST_CASE("Test Circle Size", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 85, 5);
  canvas.Circle(60, 65, 25);
  canvas.Circle(140, 40, 50);
  canvas.Circle(300, 15, 75);
  canvas.SavePNG("circle_size.png");

}

TEST_CASE("Test Circle Thickness", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20, "white", "black", 1);
  canvas.Circle(80, 40, 20, "white", "black", 3);
  canvas.Circle(140, 40, 20, "white", "black", 5);
  canvas.Circle(200, 40, 20, "white", "black", 7);

  
  canvas.SavePNG("circle_thickness.png");

}
/*
TEST_CASE("Test Rectangle Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20);
  canvas.Circle(230, 40, 20);
  canvas.Circle(440, 40, 20);
  canvas.Circle(20, 440, 20);
  canvas.Circle(230, 440, 20);
  canvas.Circle(440, 440, 20);
  
  canvas.SavePNG("rect_location.png");

}

TEST_CASE("Test Rectangle Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20);
  canvas.Circle(230, 40, 20);
  canvas.Circle(440, 40, 20);
  canvas.Circle(20, 440, 20);
  canvas.Circle(230, 440, 20);
  canvas.Circle(440, 440, 20);
  
  canvas.SavePNG("rect_location.png");

}

TEST_CASE("Test Text Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.text(20, 40, "upper left");
  canvas.text(230, 40, "upper middle");
  canvas.text(440, 40, "top right");
  canvas.text(20, 440, "bottom left");
  canvas.text(230, 440, "bottom middle");
  canvas.text(440, 440, "bottom right");
  
  canvas.SavePNG("text_location.png");

}
*/