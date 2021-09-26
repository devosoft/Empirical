/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file Card.cpp
 */

#include <iostream>

#include "emp/prefab/Card.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  // Plain Card
  emp::prefab::Card pCard("STATIC");
  pCard.AddHeaderContent("Plain card");
  pCard << "Plain body content";
  doc << pCard;

  // Collapsible Card, default open
  emp::prefab::Card openCard("INIT_OPEN", true);
  // Header content with bootstrap link properties
  openCard.AddHeaderContent("Open card", true);
  openCard << "Open body content <br> Glyphs <br> Linked title";
  openCard.SetOnToggle([](){
    std::cout << "Card Toggled!" << std::endl;
  });
  doc << openCard;

  // Collapsible Card, default closed
  emp::prefab::Card closedCard("INIT_CLOSED", false);
  closedCard.AddHeaderContent("Closed card");
  closedCard << "Closed body content <br>No Glyphs <br> Plain title";
  closedCard.SetOnToggle(openCard.GetOnToggle());
  doc << closedCard;

}
