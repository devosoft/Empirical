
#include "../../web/web_init.h"
#include "../../web/Animate.h"

#include "../../tools/BitSet.h"
#include "../../tools/Random.h"

#include "../../web/JSWrap.h"
#include "../../web/emfunctions.h"

#include "../../web/canvas_utils.h"
#include "../../web/web.h"
#include "../../web/Document.h"

#include "../../web/d3/selection.h"
#include "../../web/d3/scales.h"
#include "../../web/d3/axis.h"

#include "../../evo/NK-const.h"
#include "../../evo/OEE.h"
#include "../../evo/World.h"
#include "../../evo/StatsManager.h"
#include "../../web/d3/visualizations.h"
#include "../../evo/evo_animation.h"

#include <iostream>
#include <functional>


emp::web::SpatialGridTreeVisualization<> lineage_viz(6000, 5000);
emp::web::Document doc("lineage_viz");

int main() {
  doc << lineage_viz;
  lineage_viz.LoadDataFromFile("test.json");
}
