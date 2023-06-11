/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file Sudoku.cpp
 */

#include "emp/base/vector.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

class SudokuBoard : public UI::Div {
private:
  std::string name;
  UI::Table table;
  emp::vector<char> state;

  void UpdateCell(size_t r, size_t c) {
    auto cell = table.GetCell(r, c);
    cell.SetCSS("border", "1px solid black"); // Make a think black line between cells
    cell.SetCSS("width", "45px");             // Make cells all 45px by 45px squares
    cell.SetCSS("height", "45px");

    emp_assert(state.size() == 81);
    const char cur_symbol = state[r*9+c];
    switch (cur_symbol) {
    case '-':   // Empty cell!
    case ' ':
      break;
    case '*':   // List of options!
      // If we're adding a list of all nine options, use a smaller font.
      cell.SetCSS("font", "15px Calibri, sans-serif")
        << "<center>1 2 3<br>4 5 6<br>7 8 9</center>";
      break;
    default:    // Locked cell!
      cell <<  "<center>" << cur_symbol << "</center>";
    };
  }
public:
  SudokuBoard(const std::string & in_name)
  : UI::Div(in_name)
  , name(in_name)        // HTML id for the slate
  , table(9,9)           // Build a 9x9 table for actual puzzle
  {
    // Setup background slate to draw board on.
    SetCSS("border", "3px solid black")   // Put a think boarder on the background
      .SetCSS("border-radius", "10px")    // Round the corners
      .SetBackground("#F0F0FF")           // A light-blue background for the board
      .SetWidth(413);                     // Make background correct size for board

    // Add the table.
    *this << table;

    table.SetCSS("border-collapse", "collapse")     // Remove gaps between cells in the puzzle
      .SetCSS("font", "35px Calibri, sans-serif");  // Use a nice 35pt font for numbers in cells

    // Setup some values to go in the puzzle.
    // Note that '-' is an empty cell, and '*' shows all cell options.
    state = {{ '-','1','2', '3','4','5', '6','7','8',
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
    for (size_t r = 0; r < 9; r++) {
      for (size_t c = 0; c < 9; c++) {
        auto cell = table.GetCell(r,c);
        cell.On("mousedown", [cell,r,c]() mutable {
          // doc.Div("table_bg").SetBackground("red");
          // cell.SetCSS("BackgroundColor", "grey");
          cell.Clear();
        });
        cell.On("mouseup", [this,r,c]() mutable {
          // cell.SetCSS("BackgroundColor", "white");
          // doc.Div("table_bg").SetBackground("white");
          state[r*9+c] = '*';
          UpdateCell(r,c);
        });
        // cell.On("mousemove", [cell]() mutable {
        //   cell.SetCSS("BackgroundColor", "pink");
        //   doc.Div("table_bg").SetBackground("pink");
        // });

        UpdateCell(r,c);
      }
    }

  }
};

SudokuBoard board("test_board");

int main()
{
  doc << "<h1>Sudoku!</h1>";
  doc << board;
}
