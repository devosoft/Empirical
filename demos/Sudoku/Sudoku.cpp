/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file Sudoku.cpp
 */

#include "emp/base/array.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

struct CellState {
  char state;                  // -1 = UNKNOWN; 0-8 is associated value.
  emp::array<int, 9> options;  // Is each state possible?

  CellState() : state(-1) { options.fill(true); }

  CellState & operator=(int s) { state = s; return *this; }
};

class SudokuBoard : public UI::Div {
private:
  std::string name;
  UI::Table table;
  emp::array<CellState, 81> states;
  emp::array<char,9> symbols;
  int cur_row;
  int cur_col;

public:
  SudokuBoard(const std::string & in_name)
  : UI::Div(in_name)
  , name(in_name)        // HTML id for the slate
  , table(9,9)           // Build a 9x9 table for actual puzzle
  , symbols({{'1','2','3','4','5','6','7','8','9'}})
  , cur_row(-1), cur_col(-1)
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

    // Make regions have a dark background.
    table.GetColGroup(0).SetSpan(3).SetCSS("border", "3px solid black");
    table.GetColGroup(3).SetSpan(3).SetCSS("border", "3px solid black");
    table.GetColGroup(6).SetSpan(3).SetCSS("border", "3px solid black");
    table.GetRowGroup(0).SetSpan(3).SetCSS("border", "3px solid black");
    table.GetRowGroup(3).SetSpan(3).SetCSS("border", "3px solid black");
    table.GetRowGroup(6).SetSpan(3).SetCSS("border", "3px solid black");

    Update();
  }

  void UpdateCell(size_t r, size_t c) {
    auto cell = table.GetCell(r, c);
    cell.SetCSS("border", "1px solid black"); // Make a think black line between cells
    cell.SetCSS("width", "45px");             // Make cells all 45px by 45px squares
    cell.SetCSS("height", "45px");

    const char symbol_id = states[r*9+c].state;
    if (r == cur_row && c == cur_col) {
      cell.SetCSS("font", "15px Calibri, sans-serif")
        << "<center>1 2 3<br>4 5 6<br>7 8 9</center>";
    }
    else if (symbol_id >= 0) {
      cell <<  "<center>" << symbols[symbol_id] << "</center>";
    }
    else {} // Otherwise leave this cell blank! (for now)

    //   // If we're adding a list of all nine options, use a smaller font.
  }

  void Update() {
    // Add all symbols to the table.
    for (size_t r = 0; r < 9; r++) {
      for (size_t c = 0; c < 9; c++) {
        auto cell = table.GetCell(r,c);
        // cell.On("mousedown", [cell,r,c]() mutable {
        //   // doc.Div("table_bg").SetBackground("red");
        //   // cell.SetCSS("BackgroundColor", "grey");
        //   cell.Clear();
        // });
        // cell.On("mouseup", [this,r,c]() mutable {
        //   // cell.SetCSS("BackgroundColor", "white");
        //   // doc.Div("table_bg").SetBackground("white");
        //   // states[r*9+c] = '*';
        //   UpdateCell(r,c);
        // });
        cell.On("mouseenter", [this,cell,r,c]() mutable {
          if (cur_row == r && cur_col == c) return;
          int old_row = cur_row;
          int old_col = cur_col;
          auto old_cell = cell.GetCell(old_row, old_col);
          cur_row = r; cur_col = c;

          cell.ClearStyle();
          cell.ClearChildren();
          old_cell.ClearStyle();
          old_cell.ClearChildren();
          UpdateCell(r,c);
          UpdateCell(old_row,old_col);
          doc.Div(name).Redraw();
        });

        UpdateCell(r,c);
      }
    }
  }

  CellState & operator[](int id) { return states[id]; }
  const CellState & operator[](int id) const { return states[id]; }
};

SudokuBoard board("test_board");

int main()
{
  doc << "<h1>Sudoku!</h1>";
  doc << board;

  // Setup some values to go in the puzzle.
  emp::array<int,81> states = {{ -1, 1, 2,   3, 4, 5,   6, 7,-1,
                                  5, -1, 4,  6, 0, 8,   1, 2, 3,
                                  3,  8,-1,  1, 7, 2,   0, 5, 4,

                                  8, 2, 0,   7, 3, 6,   4, 1, 5,
                                  1, 5, 3,   8, 2, 4,   7, 6, 0,
                                  6, 4, 7,   0, 5, 1,   3, 8, 2,

                                  7, 0, 1,   5, 8, 3,   2, 4, 6,
                                  4, 6, 5,   2, 1, 0,   8,-1, 7,
                                  2, 3, 8,   4, 6, 7,   5, 0, 1
    }};

  for (int i = 0 ; i < 81; i++) board[i] = states[i];
  board.Update();
}
