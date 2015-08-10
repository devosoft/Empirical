#ifndef EMP_WEB_SLATE_H
#define EMP_WEB_SLATE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Slate widget.
//

class Button;
class Canvas;
class Image;
class Selector;
class Table;

#include "Text.h"
#include "Widget.h"

namespace emp {
namespace web {

  class Slate;

  class SlateInfo : public internal::WidgetInfo {
    friend Slate;
  protected:
    std::map<std::string, Widget> widget_dict;   // By-name lookup for widgets
    
    SlateInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
    SlateInfo(const SlateInfo &) = delete;               // No copies of INFO allowed
    SlateInfo & operator=(const SlateInfo &) = delete;   // No copies of INFO allowed
    virtual ~SlateInfo() { ; }

    // Return a text element for appending.  Use the last element unless there are no elements,
    // the last element is not text, or it is not appendable (instead, build a new one).
    web::Text & GetTextWidget() {
      // If the final element is not text, add one.
      if (children.size() == 0
          || children.back().IsText() == false
          || children.back().AppendOK() == false)  {
        AddChild(Text());
      }
      return (Text &) children.back();
    }

    bool IsRegistered(const std::string & test_name) {
      return (widget_dict.find(test_name) != widget_dict.end());
    }

    Widget & GetRegistered(const std::string & find_name) {
      emp_assert(IsRegistered(find_name), find_name, widget_dict.size());
      return widget_dict[find_name];
    }

    void Register(Widget & new_widget) override {
      // Make sure name is not already used
      emp_assert(IsRegistered(new_widget.GetID()) == false, new_widget.GetID());

      widget_dict[new_widget.GetID()] = new_widget;   // Track widget by name
      if (parent) parent->Register(new_widget);       // Also register in parent, if available
    }

    // Add additional children on to this element.
    Widget Append(const std::string & text) override {
      return GetTextWidget() << text;
    }
    Widget Append(const std::function<std::string()> & in_fun) override {
      return GetTextWidget() << in_fun;
    }

    Widget Append(Widget info) override { AddChild(info); return info; }
    
    // All derived widgets must suply a mechanism for providing associated HTML code.
    virtual void GetHTML(std::stringstream & HTML) override {
      HTML.str("");       // Clear the current text.

      // Loop through all children and build a span element for each to replace.
      for (Widget & w : children) {
        HTML << "<span id=\'" << w.GetID() << "'></span>";  // Span element for current widget.
      }
    }

  public:
    virtual std::string GetType() override { return "web::SlateInfo"; }
  };



  class Slate : public internal::WidgetFacet<Slate> {
  protected:
    // Get a properly cast version of indo.
    SlateInfo * Info() { return (SlateInfo *) info; }

  public:
    Slate(const std::string & in_name) : WidgetFacet(in_name) {
      // When a name is provided, create an associated Widget info.
      info = new SlateInfo(in_name);
    }
    Slate(const Slate & in) : WidgetFacet(in) { ; }
    Slate(const Widget & in) : WidgetFacet(in) { ; }
    ~Slate() { ; }

    virtual bool IsSlate() const { return true; }
    using INFO_TYPE = SlateInfo;
   
    // Methods to look up previously created elements, by type.
    Widget & Find(const std::string & test_name) {
       return Info()->GetRegistered(test_name);
    }

    Button &   FindButton  (const std::string & in_id) { return (Button &)   Find(in_id); }
    Canvas &   FindCanvas  (const std::string & in_id) { return (Canvas &)   Find(in_id); }
    Image &    FindImage   (const std::string & in_id) { return (Image &)    Find(in_id); }
    Selector & FindSelector(const std::string & in_id) { return (Selector &) Find(in_id); }
    Slate &    FindSlate   (const std::string & in_id) { return (Slate &)    Find(in_id); }
    Table &    FindTable   (const std::string & in_id) { return (Table &)    Find(in_id); }
    Text &     FindText    (const std::string & in_id) { return (Text &)     Find(in_id); }

  };

};
};

#endif
