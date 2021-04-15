#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"


#include "emp/web/Canvas.hpp"

#ifdef EMP_HAS_SFML

TEST_CASE("Test Canvas Attributes", "[native]") {
    emp::web::Canvas canvas{ 500, 600 };
    REQUIRE(canvas.GetWidth() == 500);
    REQUIRE(canvas.GetHeight() == 600);

    canvas.Draw(emp::Circle(20, 40, 20));
    canvas.Draw(emp::Circle(20, 40, 20), "white", "black");
    canvas.Clear("black");
    canvas.DownloadPNG();
}

TEST_CASE("Test Circle Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20);
  canvas.Circle(230, 40, 20);
  canvas.Circle(440, 40, 20);
  canvas.Circle(20, 440, 20, "white", "black");
  canvas.Circle(230, 440, 20, "white", "black");
  canvas.Circle(440, 440, 20, "white", "black");
  
  canvas.SavePNG("test_circle_location.png");

}

TEST_CASE("Test Circle Locations By Point", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.Circle(emp::Point(40,60), 20);
  canvas.Circle(emp::Point(250,60), 20);
  canvas.Circle(emp::Point(460,60), 20);
  canvas.Circle(emp::Point(40,460), 20, "white", "black");
  canvas.Circle(emp::Point(250,460), 20, "white", "black");
  canvas.Circle(emp::Point(460,460), 20, "white", "black");
  
  canvas.SavePNG("test_circle_location2.png");

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

  canvas.SavePNG("test_circle_color.png");

}

TEST_CASE("Test Circle Size", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 85, 5);
  canvas.Circle(60, 65, 25);
  canvas.Circle(140, 40, 50);
  canvas.Circle(300, 15, 75);

  canvas.SavePNG("test_circle_size.png");

}

TEST_CASE("Test Circle Thickness", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Circle(20, 40, 20, "white", "black", 1);
  canvas.Circle(80, 40, 20, "white", "black", 3);
  canvas.Circle(140, 40, 20, "white", "black", 5);
  canvas.Circle(200, 40, 20, "white", "black", 7);

  canvas.SavePNG("test_circle_thickness.png");

}

TEST_CASE("Test Rectangle Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Rect(20, 40, 20, 30);
  canvas.Rect(230, 40, 20, 30);
  canvas.Rect(440, 40, 20, 30);
  canvas.Rect(20, 440, 20, 30);
  canvas.Rect(230, 440, 20, 30);
  canvas.Rect(440, 440, 20, 30);

  canvas.SavePNG("test_rect_location.png");

}

TEST_CASE("Test Rectangle Locations by Point", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.Rect(emp::Point(20, 40), 20, 30);
  canvas.Rect(emp::Point(230, 40), 20, 30);
  canvas.Rect(emp::Point(440, 40), 20, 30);
  canvas.Rect(emp::Point(20, 440), 20, 30, "white", "black");
  canvas.Rect(emp::Point(230, 440), 20, 30, "white", "black");
  canvas.Rect(emp::Point(440, 440), 20, 30, "white", "black");

  canvas.SavePNG("test_rect_location2.png");

}

TEST_CASE("Test Rectangle Colors", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Rect(20, 40, 20, 30, "white", "black");
  canvas.Rect(160, 40, 20, 30, "white", "red");
  canvas.Rect(300, 40, 20, 30, "white", "yellow");
  canvas.Rect(440, 40, 20, 30, "white", "blue");
  canvas.Rect(20, 80, 20, 30, "black", "white");
  canvas.Rect(160, 80, 20, 30, "red", "black");
  canvas.Rect(300, 80, 20, 30, "yellow", "black");
  canvas.Rect(440, 80, 20, 30, "blue", "black");

  canvas.SavePNG("test_rect_color.png");

}

TEST_CASE("Test Rectangle Size/Thickness", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Rect(20, 85, 10, 5, "white", "black", 1);
  canvas.Rect(60, 65, 30, 25, "white", "black", 3);
  canvas.Rect(140, 40, 55, 50, "white", "black", 5);
  canvas.Rect(300, 15, 80, 75, "white", "black", 7);

  canvas.SavePNG("test_rect_size.png");

}

TEST_CASE("Test Line Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.Line(20, 40, 20, 60);
  canvas.Line(230, 40, 230, 60);
  canvas.Line(440, 40, 440, 60);
  canvas.Line(20, 400, 40, 400, "black", "black");
  canvas.Line(230, 400, 250, 400, "black", "black");
  canvas.Line(440, 400, 460, 400, "black", "black");

  canvas.SavePNG("test_line_location.png");

}

TEST_CASE("Test Line Locations by Point", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.Line(emp::Point(20, 40), emp::Point(20, 60));
  canvas.Line(emp::Point(230, 40), emp::Point(230, 60));
  canvas.Line(emp::Point(440, 40), emp::Point(440, 60));
  canvas.Line(emp::Point(20, 400), emp::Point(40, 400), "black", "black");
  canvas.Line(emp::Point(230, 400), emp::Point(250, 400), "black", "black");
  canvas.Line(emp::Point(440, 400), emp::Point(460, 400), "black", "black");

  canvas.SavePNG("test_line_location2.png");

}

TEST_CASE("Test Text Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Text(20, 40, "upper left");
  canvas.Text(220, 40, "upper middle");
  canvas.Text(420, 40, "upper right");
  canvas.Text(20, 440, "bottom left", "black", "black");
  canvas.Text(220, 440, "bottom middle", "black", "black");
  canvas.Text(420, 440, "bottom right", "black", "black");

  canvas.SavePNG("test_text_location.png");

}

TEST_CASE("Test Text Locations by Point", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.Text(emp::Point(20, 40), "upper left");
  canvas.Text(emp::Point(220, 40), "upper middle");
  canvas.Text(emp::Point(420, 40), "upper right");
  canvas.Text(emp::Point(20, 440), "bottom left", "black", "black");
  canvas.Text(emp::Point(220, 440), "bottom middle", "black", "black");
  canvas.Text(emp::Point(420, 440), "bottom right", "black", "black");

  canvas.SavePNG("test_text_location2.png");

}

TEST_CASE("Test Text Color", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.Text(60, 40, "black", "black", "black", 20);
  canvas.Text(60, 80, "white", "white", "black", 20);
  canvas.Text(60, 120, "red", "red", "black", 20);
  canvas.Text(60, 160, "yellow", "yellow", "black", 20);
  canvas.Text(60, 200, "blue", "blue", "black", 20);

  canvas.SavePNG("test_text_color.png");

}

TEST_CASE("Test CenterText Locations", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };
  
  canvas.CenterText(20, 40, "upper left");
  canvas.CenterText(220, 40, "upper middle");
  canvas.CenterText(420, 40, "upper right");
  canvas.CenterText(20, 440, "bottom left", "black", "black");
  canvas.CenterText(220, 440, "bottom middle", "black", "black");
  canvas.CenterText(420, 440, "bottom right", "black", "black");
  
  canvas.SavePNG("test_centertext_location.png");

}

TEST_CASE("Test CenterText Locations by Point", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  canvas.CenterText(emp::Point(20, 40), "upper left");
  canvas.CenterText(emp::Point(220, 40), "upper middle");
  canvas.CenterText(emp::Point(420, 40), "upper right");
  canvas.CenterText(emp::Point(20, 440), "bottom left", "black", "black");
  canvas.CenterText(emp::Point(220, 440), "bottom middle", "black", "black");
  canvas.CenterText(emp::Point(420, 440), "bottom right", "black", "black");

  canvas.SavePNG("test_centertext_location2.png");

}

#endif
