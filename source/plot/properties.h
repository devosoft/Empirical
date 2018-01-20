#ifndef PLOT_PROPERTIES_H
#define PLOT_PROPERTIES_H

#include <utility>
#include "data.h"

namespace emp {
  namespace plot {
    DEFINE_PROPERTY(Stroke, stroke);
    DEFINE_PROPERTY(StrokeWeight, strokeWeight);
    DEFINE_PROPERTY(Fill, fill);
    DEFINE_PROPERTY(PointSize, pointSize);
  }  // namespace plot
}  // namespace emp
#endif  // PLOT_PROPERTIES_H
