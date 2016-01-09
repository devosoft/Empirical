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

  UI::Slate table_bg("table_bg");                // Background slate to draw board on.
  table_bg.SetCSS("border", "3px solid black")   // Put a think boarder on the background
    .SetCSS("border-radius", "10px")             // Round the corners
    .SetBackground("#F0F0FF")                    // A light-blue background for the board
    .SetWidth(413);                              // Make backgroung correct size for board
  
  UI::Table table(9,9);                          // Build a 9x9 table for actual puzzle
  table_bg << table;                             // Attach the puzzle to the background
  doc << table_bg;                               // Attach the background to the web page
    
  table.SetCSS("border-collapse", "collapse")    // Remove gaps between cells in the puzzle
    .SetCSS("font", "35px Calibri, sans-serif")  // Use a nice 35pt font for numbers in cells
    .CellsCSS("border", "1px solid black")       // Make a think black line between cells
    .CellsCSS("width", "45px")                   // Make cells all 45px by 45px squares
    .CellsCSS("height", "45px");

  // Setup some values to go in the puzzle.
  // Note that '-' is an empty cell, and '*' shows all cell options.
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

  // Add all symbols to the table.
  for (int r = 0; r < 9; r++) {
    for (int c = 0; c < 9; c++) {
      const char cur_symbol = puzzle[r*9+c];
      switch (cur_symbol) {
      case '-':   // Empty cell!
      case ' ':
        break;
      case '*':   // List of options!
        // If we're adding a list of all nine options, use a smaller font.
        table.GetCell(r,c).SetCSS("font", "15px/15px Calibri, sans-serif")
          << "<center>1 2 3<br>4 5 6<br>7 8 9</center>";
        break;
      default:    // Locked cell!
        table.GetCell(r,c) <<  "<center>" << cur_symbol << "</center>";
      };
    }
  }
}
