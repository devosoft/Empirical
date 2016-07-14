
#include "../../web/web_init.h"
#include "../../tools/BitSet.h"
#include "../../web/Animate.h"
#include "../../emtools/JSWrap.h"
#include "../../web/canvas_utils.h"
#include "../../web/web.h"
#include "../../web/Document.h"
#include "../../emtools/emfunctions.h"

#include "../../evo/visualization_utils.h"
#include "../../evo/evo_animation.h"
#include "../../../d3-emscripten/selection.h"
#include "../../../d3-emscripten/scales.h"
#include "../../../d3-emscripten/axis.h"


#include <iostream>
#include <functional>

#include "../../evo/NK-const.h"
#include "../../evo/OEE.h"
#include "../../evo/World.h"
#include "../../tools/Random.h"
#include "../../evo/StatsManager.h"



emp::web::LineageVisualization lineage_viz(5000, 1000);
emp::web::Document doc("lineage_viz");

int main() {
  doc << lineage_viz;
  lineage_viz.LoadDataFromFile("test.json");
}
