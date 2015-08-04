#ifndef EMP_WEB_SLATE_H
#define EMP_WEB_SLATE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Slate widget.
//

#include "Text.h"
#include "Widget.h"

namespace emp {
namespace web {

  class Slate;

  class SlateInfo : public internal::WidgetInfo {
    friend Slate;
  protected:
    std::map<std::string, internal::Widget> widget_dict;   // By-name lookup for widgets
    
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

    internal::Widget & GetRegistered(const std::string & find_name) {
      emp_assert(IsRegistered(find_name));
      return widget_dict[find_name];
    }

    void Register(internal::Widget & new_widget) override {
      // Make sure name is not already used
      emp_assert(IsRegistered(new_widget.GetID()) == false);

      widget_dict[new_widget.GetID()] = new_widget;   // Track widget by name
      if (parent) parent->Register(new_widget);       // Also register in parent, if available
    }

    // Add additional children on to this element.
    internal::Widget Append(const std::string & text) override {
      return GetTextWidget() << text;
    }
    internal::Widget Append(const std::function<std::string()> & in_fun) override {
      return GetTextWidget() << in_fun;
    }

    // Default to passing specialty operators to parent.
    // Widget & Append(emp::web::Button & info) override { return AddChild(info); }
    // Widget & Append(emp::web::Canvas & info) override { return AddChild(info); }
    // Widget & Append(emp::web::Image & info) override { return AddChild(info); }
    // Widget & Append(emp::web::Selector & info) override { return AddChild(info); }
    // Widget & Append(emp::web::Slate & info) override { return AddChild(info); }
    // Widget & Append(emp::web::Table & info) override { return AddChild(info); }
    // Widget & Append(emp::web::Text & info) override { return AddChild(info); }
    
    // All derived widgets must suply a mechanism for providing associated HTML code.
    virtual void GetHTML(std::stringstream & HTML) override {
      HTML.str("");       // Clear the current text.

      // Loop through all children and build a span element for each to replace.
      for (internal::Widget & w : children) {
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
    ~Slate() { ; }

    virtual bool IsSlate() const { return true; }

    // Methods to look up previously created elements, by type.
    // web::Button & Button(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Button *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Button&>( Info()->GetRegistered(test_name) );
    // }
    // web::Canvas & Canvas(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Canvas *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Canvas&>( Info()->GetRegistered(test_name) );
    // }
    // web::Image & Image(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Image *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Image&>( Info()->GetRegistered(test_name) );
    // }
    // web::Selector & Selector(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Selector *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Selector&>( Info()->GetRegistered(test_name) );
    // }
    // web::Slate & Slate(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Slate *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Slate&>( Info()->GetRegistered(test_name) );
    // }
    // web::Table & Table(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Table *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Table&>( Info()->GetRegistered(test_name) );
    // }
    // web::Text & Text(const std::string & test_name) {
    //   emp_assert(dynamic_cast<Text *>( &(Info()->GetRegistered(test_name)) ) != NULL);
    //   return dynamic_cast<Text&>( Info()->GetRegistered(test_name) );
    // }


  };

};
};

#endif
