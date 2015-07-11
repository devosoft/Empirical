#include "../../tools/Random.h"
#include "../../UI/UI.h"
#include "../../UI/canvas_utils.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

int main() {
  UI::Initialize();

  emp::Random random;

  // Draw a random bitmap onto a canvas.

  auto & canvas = doc.AddCanvas(300, 300, "can");
  //UI::Draw(canvas, emp::Circle<>(emp::Point<>(20, 20), 10));
  
  emp::BitMatrix<10,10> matrix;
  for (int x = 0; x < 10; x++) {
    for (int y = 0; y < 10; y++) {
      if (random.P(0.5)) matrix.Set(x,y);
    }
  }

  UI::Draw(canvas, matrix, 300, 300);


  auto & canvas2 = doc.AddCanvas(300, 300, "can2");
  (void) canvas2;

  doc.Update();
}
