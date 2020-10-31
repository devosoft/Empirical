#include "../../UI/UI.h"
#include "../../web/emfunctions.h"

namespace UI = emp::UI;
UI::Document doc("emp_base");

int main() {

  UI::Initialize();

  doc << "<h1>This is my header!</h1>"
      << "This is some text...";

  doc.Update();
}

