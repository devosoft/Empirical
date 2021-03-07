/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file  FacetedWidget.h
 *  @brief The FacetedWidget class is used to access the protected methods of WidgetFacet such
 *          as SetCSS and SetAttr. An example of its use can be found in include/emp/prefab/Collapse.h
 *
 */

#include "../web/Widget.hpp"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace emp{
namespace web{
  namespace internal{
    class FacetedWidget : public internal::WidgetFacet<FacetedWidget>
    {
      public:
      using base_t = internal::WidgetFacet<FacetedWidget>;
      using INFO_TYPE = internal::WidgetInfo;
      FacetedWidget(const Widget & widget) : base_t(widget){
      }
    };
  }
}
}

#endif
