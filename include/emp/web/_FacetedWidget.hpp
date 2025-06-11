/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/_FacetedWidget.hpp
 * @brief The FacetedWidget class is used to access the protected methods of WidgetFacet such
 * as SetCSS and SetAttr. An example of its use can be found in include/emp/prefab/Collapse.h
 *
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_FACETED_WIDGET_HPP_GUARD
#define INCLUDE_EMP_WEB_FACETED_WIDGET_HPP_GUARD

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

#endif // #ifndef EMP_WEB__FACETEDWIDGET_HPP_INCLUDE
