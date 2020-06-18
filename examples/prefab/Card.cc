//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "prefab/Card.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
    // Plain Card
    emp::Card pCard(emp::Card::Collapse::none);
    pCard.AddHeaderContent("Plain card");
    pCard.AddBodyContent("Plain body content");
    doc << pCard.GetDiv();

    // Collapsible Card, default open
    emp::Card openCard(emp::Card::Collapse::open, true);
    // Header content with bootstrap link properties
    openCard.AddHeaderContent("Open card", true);
    openCard.AddBodyContent("Open body content <br> Glyphs <br> Linked title");
    doc << openCard.GetDiv();

    // Collapsible Card, default closed
    emp::Card closedCard(emp::Card::Collapse::closed, false);
    closedCard.AddHeaderContent("Closed card");
    closedCard.AddBodyContent("Closed body content <br>No Glyphs <br> Plain title");
    doc << closedCard.GetDiv();
}
