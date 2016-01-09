// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <vector>
#include "../../web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << "<h1>Sudoku!</h1>";

  UI::Slate table_bg("table_bg");
  table_bg.SetCSS("border", "3px solid black")
    .SetCSS("border-radius", "10px")
    .SetBackground("#F0F0FF")
    .SetWidth(413);
  
  UI::Table table(9,9);
  table_bg << table;
  doc << table_bg;
    
  table.SetCSS("border-collapse", "collapse")
    .SetCSS("font", "35px Calibri, sans-serif")
    .CellsCSS("border", "1px solid black")  // Each cell should have a light background.
    .CellsCSS("width", "45px")
    .CellsCSS("height", "45px");
  
  // std::vector<int> puzzle = {{ '9','1','2', '3','4','5', '6','7','8', 
  //                              '5','7','4', '6','9','8', '1','2','3', 
  //                              '3','8','6', '1','7','2', '9','5','4', 
  //                              '8','2','9', '7','3','6', '4','1','5', 
  //                              '1','5','3', '8','2','4', '7','6','9', 
  //                              '6','4','7', '9','5','1', '3','8','2', 
  //                              '7','9','1', '5','8','3', '2','4','6', 
  //                              '4','6','5', '2','1','9', '8','3','7', 
  //                              '2','3','8', '4','6','7', '5','9','1'
  //   }};
  std::vector<int> puzzle = {{ '-','1','2', '3','4','5', '6','7','8', 
                               '5','-','4', '6','9','8', '1','2','3', 
                               '3','8','-', '1','7','2', '9','5','4', 
                               '8','2','9', '7','3','6', '4','1','5', 
                               '1','5','3', '8','2','4', '7','6','9', 
                               '6','4','7', '9','5','1', '3','8','2', 
                               '7','9','1', '5','8','3', '2','4','6', 
                               '4','6','5', '2','1','9', '8','*','7', 
                               '2','3','8', '4','6','7', '5','9','1'
    }};

          // Make regions have a dark background.
  table.GetColGroup(0).SetSpan(3).SetCSS("border", "3px solid black");
  table.GetColGroup(3).SetSpan(3).SetCSS("border", "3px solid black");
  table.GetColGroup(6).SetSpan(3).SetCSS("border", "3px solid black");
  table.GetRowGroup(0).SetSpan(3).SetCSS("border", "3px solid black");
  table.GetRowGroup(3).SetSpan(3).SetCSS("border", "3px solid black");
  table.GetRowGroup(6).SetSpan(3).SetCSS("border", "3px solid black");
  
  for (int r = 0; r < 9; r++) {
    for (int c = 0; c < 9; c++) {
      const char cur_symbol = puzzle[r*9+c];
      switch (cur_symbol) {
      case '-':
        break;
      case '*':
        table.GetCell(r,c).SetCSS("font", "15px/15px Calibri, sans-serif") << "<center>1 2 3<br>4 5 6<br>7 8 9</center>";
        break;
      default:
        table.GetCell(r,c) <<  "<center>" << cur_symbol << "</center>";
      };
    }
  }
}
