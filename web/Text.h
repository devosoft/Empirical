#ifndef EMP_WEB_TEXT_H
#define EMP_WEB_TEXT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Text widget.
//

#include "../tools/DynamicStringSet.h"

#include "Widget.h"

namespace emp {
namespace web {

  class Text : public internal::WidgetFacet<Text> {
    friend class TextInfo;
  protected:

  class TextInfo : public internal::WidgetInfo {
    friend Text;
  protected:
    DynamicStringSet strings;

    TextInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
    TextInfo(const TextInfo &) = delete;               // No copies of INFO allowed
    TextInfo & operator=(const TextInfo &) = delete;   // No copies of INFO allowed
    virtual ~TextInfo() { ; }

    virtual bool IsTextInfo() const override { return true; }

    Widget Append(const std::string & in_text) override;
    Widget Append(const std::function<std::string()> & in_fun) override;

    // All derived widgets must suply a mechanism for providing associated HTML code.
    virtual void GetHTML(std::stringstream & HTML) override {
      HTML.str("");                         // Clear the current text.
      HTML << "<span id=\'" << id << "'>"   // Initial span tag to keep id.
           << strings                       // Save the current value of all of the strings.      
           << "</span>";                    // Close span tag.
    }

  public:
    virtual std::string GetType() override { return "web::TextInfo"; }
  };  // End of TextInfo


    // Get a properly cast version of indo.
    TextInfo * Info() { return (TextInfo *) info; }

    Text(TextInfo * in_info) : WidgetFacet(in_info) { ; }
  public:
    Text(const std::string & in_id="") : WidgetFacet(in_id) {
      // When a name is provided, create an associated Widget info.
      info = new TextInfo(in_id);
    }
    Text(const Text & in) : WidgetFacet(in) { ; }
    Text(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsTextInfo()); }
    ~Text() { ; }

    virtual bool IsText() const { return true; }
    using INFO_TYPE = TextInfo;

    Text & Clear() { Info()->strings.Clear(); return *this; }

  };  

  Widget Text::TextInfo::Append(const std::string & in_text) {
    strings.Append(in_text);                     // Record the new string being added.
    if (state == Widget::ACTIVE) ReplaceHTML();  // If node is active, immediately redraw!
    return web::Text(this);
  }

  Widget Text::TextInfo::Append(const std::function<std::string()> & in_fun) {
    strings.Append(in_fun);                      // Record the new function being added.
    if (state == Widget::ACTIVE) ReplaceHTML();  // If node is active, immediately redraw!
    return web::Text(this);
  }

}
}

#endif
