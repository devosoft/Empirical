#include "../../UI/UI.h"
#include "../../UI/canvas_utils.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

int main() {
  UI::Initialize();

  auto & canvas = doc.AddCanvas(100, 100, "can");
  UI::Draw(canvas, emp::Circle<>(emp::Point<>(20, 20), 10));

  doc.Update();
}
