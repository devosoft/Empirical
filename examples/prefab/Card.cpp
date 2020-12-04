//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/web/web.hpp"
#include "emp/prefab/Card.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  // Plain Card
  emp::prefab::Card pCard("STATIC");
  pCard.AddHeaderContent("Plain card");
  pCard.AddBodyContent("Plain body content");
  doc << pCard;

  // Collapsible Card, default open
  emp::prefab::Card openCard("INIT_OPEN", true);
  // Header content with bootstrap link properties
  openCard.AddHeaderContent("Open card", true);
  openCard.AddBodyContent("Open body content <br> Glyphs <br> Linked title");
  doc << openCard;

  // Collapsible Card, default closed
  emp::prefab::Card closedCard("INIT_CLOSED", false);
  closedCard.AddHeaderContent("Closed card");
  closedCard.AddBodyContent("Closed body content <br>No Glyphs <br> Plain title");
  doc << closedCard;
}
