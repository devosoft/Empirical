//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "../../tools/BitSet.h"
#include "tools/Random.h"

#include "web/init.h"
#include "web/Animate.h"

#include "web/JSWrap.h"
#include "web/emfunctions.h"

#include "web/canvas_utils.h"
#include "web/web.h"
#include "web/Document.h"

#include "web/d3/selection.h"
#include "web/d3/scales.h"
#include "web/d3/axis.h"
#include "web/d3/visualizations.h"

#include "evo/NK-const.h"
#include "evo/OEE.h"
#include "evo/World.h"
#include "evo/StatsManager.h"
#include "evo/evo_animation.h"

#include <iostream>
#include <functional>


emp::web::SpatialGridTreeVisualization<> lineage_viz(10000, 10000);
emp::web::Document doc("lineage_viz");

int main() {
  lineage_viz.grid_width = 60;
  lineage_viz.grid_height = 60;
  lineage_viz.legend_cell_size = 5;
  doc << lineage_viz;
  lineage_viz.LoadDataFromFile("lineage.json");
}
