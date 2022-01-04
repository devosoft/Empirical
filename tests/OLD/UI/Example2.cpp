/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Example2.cpp
 */

#include "../../UI/UI.h"

namespace UI = emp::UI;

int myvar = 20;

void IncVar() {
  myvar++;
  UI::document.Update();
}

int main() {

  UI::Initialize();

  UI::document << "<h1>This is my header!</h2>"
               << "And here is some normal text!"
               << "<p>My variable = " << myvar
               << "<p>Live variable = " << UI::Live(myvar);

  UI::document << UI::Button( IncVar, "Inc!!!" ).Height(50).CSS("border-radius", "8px").CSS("background-color", "blue").CSS("color", "white");
  UI::document << UI::Button( [](){ myvar--; UI::document.Update(); }, "Dec!!!" ).Height(40);

  UI::document << "<br>" << UI::Table(3,3, "mytable") << "Test1!" << UI::GetCell(1,1) << "Test2!" << UI::GetCell(2,2) << "Test 3!!!!!";

  // UI::document.Table("mytable") << UI::GetCell(2,0) << "This is another test!";
  UI::document.Table("mytable").GetCell(2,0) << "This is another test!";

  UI::document << "<br>" << UI::Image("motivator.jpg").Width(400);

  myvar = 30;

  UI::document.Update();
}
