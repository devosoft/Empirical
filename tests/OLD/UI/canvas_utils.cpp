#include "../../tools/Random.h"
#include "../../UI/UI.h"
#include "../../UI/canvas_utils.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

int main() {
  UI::Initialize();

  // How big should each canvas be?
  const int w = 300;
  const int h = 300;

  emp::Random random;

  // Draw a random bitmap onto a canvas.

  auto & canvas = doc.AddCanvas(w, h, "can");
  
  emp::BitMatrix<10,10> matrix;
  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 10; y++) {
      if (random.P(0.5)) matrix.Set(x,y);
    }
  }

  UI::Draw(canvas, matrix, w, h);


  // Draw a surface with circles on it!
  using dBRAIN = int;
  using dBODY = emp::CircleBody2D<dBRAIN>;
    
  // Build the surface with bodies on it.
  emp::Surface2D<dBODY, dBRAIN> surface(w, h);
  for (int i = 0; i < 1000; i++) {
    auto new_circle = emp::Circle<>(random.GetDouble(w), random.GetDouble(h), 7);
    dBODY * new_body = new dBODY( new_circle );
    new_body->SetColorID(random.GetInt(360));  // Set color to random hue.
    surface.AddBody( new_body );
  }

  // Determine the possible colors.
  emp::vector<std::string> color_map(360);
  for (int i = 0; i < 360; i++) {
    color_map[i] = emp::to_string("hsl(", i, ",100%,50%");
  }

  // Draw the surface on a new canvas!
  auto & canvas2 = doc.AddCanvas(w, h, "can2");
  emp::UI::Draw(canvas2, surface, color_map);

  // Draw a grid on the canvas!
  const int num_rows = 50;
  const int num_cols = 20;
  emp::vector< emp::vector<int> > grid(num_rows); //  = {{0,100,200},{100,200,0},{200,0,100}};  
  for (int row = 0; row < num_rows; row++) {
    grid[row].resize(num_cols);
    for (int col = 0; col < num_cols; col++) {
      grid[row][col] = random.GetInt(360);
    }
  }

  auto & canvas3 = doc.AddCanvas(w, h, "can3");
  emp::UI::Draw(canvas3, grid, color_map);


  doc.Update();
}
