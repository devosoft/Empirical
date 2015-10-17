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

  namespace internal {
    class SlateInfo : public internal::WidgetInfo {
      friend Slate;
    protected:
      SlateInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      SlateInfo(const SlateInfo &) = delete;               // No copies of INFO allowed
      SlateInfo & operator=(const SlateInfo &) = delete;   // No copies of INFO allowed
      virtual ~SlateInfo() { ; }

      virtual bool IsSlateInfo() const override { return true; }

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
      

      // Add additional children on to this element.
      Widget Append(const std::string & text) override {
        if (!append_ok) return ForwardAppend(text);
        return GetTextWidget() << text;
      }
      Widget Append(const std::function<std::string()> & in_fun) override {
        if (!append_ok) return ForwardAppend(in_fun);
        return GetTextWidget() << in_fun;
      }
      
      Widget Append(Widget info) override {
        if (!append_ok) return ForwardAppend(info);
        AddChild(info);
        return info;
      }
      
      // All derived widgets must suply a mechanism for providing associated HTML code.
      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");       // Clear the current text.

        // Loop through all children and build a span element for each to replace.
        HTML << "<div id=\'" << id << "\'>"; // Tag to envelope Slate
        for (Widget & w : children) {
          HTML << "<span id=\'" << w.GetID() << "'></span>";  // Span element for current widget.
        }
        HTML << "</div>";
      }
      
    public:
      virtual std::string GetType() override { return "web::SlateInfo"; }
    };
  }

  class Slate : public internal::WidgetFacet<Slate> {
  protected:
    // Get a properly cast version of indo.
    internal::SlateInfo * Info() { return (internal::SlateInfo *) info; }

  public:
    Slate(const std::string & in_name) : WidgetFacet(in_name) {
      // When a name is provided, create an associated Widget info.
      info = new internal::SlateInfo(in_name);
    }
    Slate(const Slate & in) : WidgetFacet(in) { ; }
    Slate(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsSlateInfo()); }
    ~Slate() { ; }

    virtual bool IsSlate() const { return true; }
    using INFO_TYPE = internal::SlateInfo;
   
  };

}
}

#endif
