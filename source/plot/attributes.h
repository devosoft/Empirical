#ifndef EMP_PLOTTING_ATTRIBUTES
#define EMP_PLOTTING_ATTRIBUTES

#include "tools/attrs.h"

namespace emp {
  namespace plot {
    namespace attributes {
      DEFINE_ATTR(Xyz);
      DEFINE_ATTR(XyzScaled);
      DEFINE_ATTR(PointSize);
      DEFINE_ATTR(StrokeWeight);
      DEFINE_ATTR(Fill);
      DEFINE_ATTR(Stroke);
    }  // namespace attributes
  }  // namespace plot
}  // namespace emp

#endif