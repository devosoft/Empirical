//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  The Text Widget
//
//  A representation of text on a web page.  Text Widgets can be included inside of Slates
//  or Tables to cordon off a section of text (and will be automatically created when text
//  is streamed into these other widgets).  The primary benefit of explicitly creating your
//  own text widget is to control the text style.


#ifndef EMP_WEB_TEXT_H
#define EMP_WEB_TEXT_H

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
      bool append_ok;

      TextInfo(const std::string & in_id="") : internal::WidgetInfo(in_id), append_ok(true) { ; }
      TextInfo(const TextInfo &) = delete;               // No copies of INFO allowed
      TextInfo & operator=(const TextInfo &) = delete;   // No copies of INFO allowed
      virtual ~TextInfo() { ; }

      std::string TypeName() const override { return "TextInfo"; }
      virtual bool IsTextInfo() const override { return true; }

      bool AppendOK() const override { return append_ok; }
      void PreventAppend() override { append_ok = false; }

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
    const TextInfo * Info() const { return (TextInfo *) info; }

    Text(TextInfo * in_info) : WidgetFacet(in_info) { ; }
  public:
    Text(const std::string & in_id="") : WidgetFacet(in_id) {
      // When a name is provided, create an associated Widget info.
      info = new TextInfo(in_id);
    }
    Text(const Text & in) : WidgetFacet(in) { ; }
    Text(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsTextInfo()); }
    ~Text() { ; }

    using INFO_TYPE = TextInfo;

    Text & Clear() { Info()->strings.Clear(); return *this; }
  };

  Widget Text::TextInfo::Append(const std::string & text) {
    if (!append_ok) return ForwardAppend(text);  // If text widget cannot append, forward to parent.
    strings.Append(text);                        // Record the new string being added.
    if (state == Widget::ACTIVE) ReplaceHTML();  // If node is active, immediately redraw!
    return web::Text(this);
  }

  Widget Text::TextInfo::Append(const std::function<std::string()> & fun) {
    if (!append_ok) return ForwardAppend(fun);   // If text widget cannot append, forward to parent.
    strings.Append(fun);                         // Record the new function being added.
    if (state == Widget::ACTIVE) ReplaceHTML();  // If node is active, immediately redraw!
    return web::Text(this);
  }

}
}

#endif
