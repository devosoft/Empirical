#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/web/Canvas.hpp"
#include "emp/web/CanvasShape.hpp"

TEST_CASE("Test CanvasShape up", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  emp::web::CanvasPolygon poly(
    0,
    0,
    "red",
    "blue"
  );

  // add central vertex
  poly.AddPoint( 150, 150 );

  // pick other two vertices depending on direction
  poly.AddPoint( 100, 100 );
  poly.AddPoint( 200, 100 );

  // draw the polygon
  canvas.Draw(poly);

  canvas.SavePNG("up.png");

}

TEST_CASE("Test CanvasShape down", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  emp::web::CanvasPolygon poly(
    0,
    0,
    "red",
    "blue"
  );

  // add central vertex
  poly.AddPoint( 150, 150 );

  // pick other two vertices depending on direction
  poly.AddPoint( 100, 200 );
  poly.AddPoint( 200, 200 );

  // draw the polygon
  canvas.Draw(poly);

  canvas.SavePNG("down.png");

}

TEST_CASE("Test CanvasShape left", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  emp::web::CanvasPolygon poly(
    0,
    0,
    "red",
    "blue"
  );

  // add central vertex
  poly.AddPoint( 150, 150 );

  // pick other two vertices depending on direction
  poly.AddPoint( 100, 100 );
  poly.AddPoint( 100, 200 );

  // draw the polygon
  canvas.Draw(poly);

  canvas.SavePNG("left.png");

}

TEST_CASE("Test CanvasShape right", "[native]") {

  emp::web::Canvas canvas{ 500, 500 };

  emp::web::CanvasPolygon poly(
    0,
    0,
    "red",
    "blue"
  );

  // add central vertex
  poly.AddPoint( 150, 150 );

  // pick other two vertices depending on direction
  poly.AddPoint( 200, 100 );
  poly.AddPoint( 200, 200 );

  // draw the polygon
  canvas.Draw(poly);

  canvas.SavePNG("right.png");

}
