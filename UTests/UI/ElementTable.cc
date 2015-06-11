#include "../../UI/UI.h"

namespace UI = emp::UI;

int main() {

  UI::Initialize();

  UI::document << UI::Table(5,4, "test_table");
  UI::document.Table("test_table").CSS("background-color", "#880000");
  UI::document.Table("test_table").RowsCSS("background-color", "#DDDDFF");
  UI::document.Table("test_table").CellsCSS("border", "1px solid black");

  UI::document.Table("test_table").GetCell(2,2).CSS("background-color", "#008800");

  UI::document.Update();
}

