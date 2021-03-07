/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Element.hpp
 *  @brief Element Widgets maintain an ordered collection of other widgets
 *  in a HTML element with any tag (e.g., div, footer, header, p, etc.)
 *
 *  When printed to the web page, these internal widgets are presented in order.
 *
 */

#ifndef EMP_WEB_ELEMENT_H
#define EMP_WEB_ELEMENT_H


#include "Animate.hpp"
#include "Text.hpp"
#include "Widget.hpp"
#include "Div.hpp"

#include "init.hpp"

namespace emp {
namespace web {

  /// A widget to track an element in an HTML file, and all of its contents.
  class Element : public Div {
  public:
    /// @param in_tag sets the html tag for used this object (i.e., div, footer, header, p, etc.)
    /// @param in_name sets the id of this html element 
    Element(const std::string & in_tag, const std::string & in_name="")
    : Div(in_name) {
      // When a name is provided, create an associated Widget info.
      Info()->DoSetTag(in_tag);
    }
    /// Construct Element from a Widget.
    Element(const Widget & in) : Div(in) { ; }
  };

}
}

#endif
