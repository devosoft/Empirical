/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ElementTable.cpp
 */

#include "../../UI/UI.h"
#include "../../web/emfunctions.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

int main() {

  UI::Initialize();

  doc << "<h2>Test Heading...</h2>";

  doc << UI::Table(5,4, "test_table");
  doc.Table("test_table").CSS("background-color", "#880000");
  doc.Table("test_table").RowsCSS("background-color", "#DDDDFF");
  doc.Table("test_table").CellsCSS("border", "1px solid black");

  doc.Table("test_table").GetCell(2,2).CSS("background-color", "#008800");

  doc.Table("test_table") << "Test Entry!" << "!!"
                          << UI::GetCell(0,1).RowSpan(2) << "Test 2"
                          << UI::GetRow(3) << "Test 3";

  doc.Table("test_table").GetCell(3,0).ColSpan(2);

  std::stringstream os;
  doc.OK(os, true);
  doc << "<p>" << emp::text2html(os.str());

  doc << UI::Table(100,100, "grid").CSS("background-color", "black").CSS("table-layout", "fixed");
  auto & grid = doc.Table("grid");
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      std::string bg_color = "red";
      if ((i+j)%2) bg_color = "blue";
      grid.GetCell(i,j).CSS("width", 3).CSS("height", 3).CSS("background-color", bg_color);
    }
  }

  doc << UI::Button([&grid](){ grid.GetCell(1,1).CSS("background-color", "green"); doc.Update(); }, "Green!");


  doc.Update();
}
