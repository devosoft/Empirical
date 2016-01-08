// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <vector>
#include "../../web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  auto table = doc.AddTable(9,9);

  table.CellsCSS("border", "1px solid black");  // Each cell should have a light background.
  table.CellsCSS("width", "15px");
  table.CellsCSS("height", "15px");
  
  std::vector<int> puzzle = {{ 0,1,2, 3,4,5, 6,7,8, 
                               5,7,4, 6,0,8, 1,2,3, 
                               3,8,6, 1,7,2, 0,5,4, 
                               8,2,0, 7,3,6, 4,1,5, 
                               1,5,3, 8,2,4, 7,6,0, 
                               6,4,7, 0,5,1, 3,8,2, 
                               7,0,1, 5,8,3, 2,4,6, 
                               4,6,5, 2,1,0, 8,3,7, 
                               2,3,8, 4,6,7, 5,0,1
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
      const char cur_symbol = '0' + puzzle[r*9+c];
      if (cur_symbol != '-') table.GetCell(r,c) <<  "&nbsp;" << cur_symbol << "&nbsp;";
    }
  }

  table.Redraw();
}
