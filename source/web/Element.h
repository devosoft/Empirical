/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Element.h
 *  @brief Element Widgets maintain an ordered collection of other widgets
 *  in a HTML element with any tag (e.g., div, footer, header, p, etc.)
 *
 *  When printed to the web page, these internal widgets are presented in order.
 *
 */

#ifndef EMP_WEB_ELEMENT_H
#define EMP_WEB_ELEMENT_H


#include "Animate.h"
#include "Text.h"
#include "Widget.h"
#include "Div.h"

#include "init.h"

namespace emp {
namespace web {

  /// A widget to track a div in an HTML file, and all of its contents.
  class Element : public internal::WidgetFacet<Element> {
  protected:
    // Get a properly cast version of info.
    internal::DivInfo * Info() { return (internal::DivInfo *) info; }
    const internal::DivInfo * Info() const { return (internal::DivInfo *) info; }

  public:
    /// @param in_tag sets the html tag for used this object (i.e., div, footer, header, p, etc.)
    Element(const std::string & in_tag, const std::string & in_name="")
    : WidgetFacet(in_name) {
      // When a name is provided, create an associated Widget info.
      info = new internal::DivInfo(in_name, in_tag);
    }
    Element(const Element & in) : WidgetFacet(in) { ; }
    Element(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsElement()); }
    ~Element() { ; }

    using INFO_TYPE = internal::DivInfo;

    /// Where is the top of the scroll region?
    double ScrollTop() const { return Info()->scroll_top; }

    /// Set the scroll position.
    Element & ScrollTop(double in_top) { Info()->scroll_top = in_top; return *this; }

    /// Set the html tag.
    Element & SetTag(const std::string & in_tag) {
      Info()->DoSetTag(in_tag);
      return *this;
    }

    /// Clear the contents of this div.
    void Clear() { if (info) Info()->Clear(); }

    /// Remove all child widgets from this div.
    void ClearChildren() { if (info) Info()->ClearChildren(); }

    /// Determine if a specified widget is internal to this one.
    bool HasChild(const Widget & test_child) const {
      if (!info) return false;
      for (const Widget & c : Info()->m_children) if (c == test_child) return true;
      return false;
    }

    /// Remove this widget from the current document.
    void Deactivate(bool top_level) override {
      // Deactivate children before this node.
      for (auto & child : Info()->m_children) child.Deactivate(false);
      Widget::Deactivate(top_level);
    }

    /// Get an internal widget to this div, by the specified name.
    Widget & Find(const std::string & test_name) {
      emp_assert(info);
      return Info()->GetRegistered(test_name);
    }

    /// Get all direct child widgets to this div.
    emp::vector<Widget> & Children() { return Info()->m_children; }

    /// Shortcut adder for animations.
    template <class... T> web::Animate & AddAnimation(const std::string & name, T &&... args){
      web::Animate * new_anim = new web::Animate(std::forward<T>(args)...);
      emp_assert(Info()->anim_map.find(name) == Info()->anim_map.end());  // Make sure not in map already!
      Info()->anim_map[name] = new_anim;
      return *new_anim;
    }

    // A quick way to retrieve Animate widgets by name.
    web::Animate & Animate (const std::string & in_id) { return *(Info()->anim_map[in_id]); }
  };

}
}

#endif
