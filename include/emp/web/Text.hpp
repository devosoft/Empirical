/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Text.hpp
 *  @brief Specs for the Text widget.
 *
 *  A representation of text on a web page.  Text Widgets can be included inside of Divs or
 *  Tables to cordon off a section of text (and will be automatically created when text is
 *  streamed into these other widgets).  The primary benefit of explicitly creating your
 *  own text widget is to control the text style.
 */

#ifndef EMP_WEB_TEXT_H
#define EMP_WEB_TEXT_H

#include "../datastructs/DynamicString.hpp"

#include "Widget.hpp"

namespace emp {
namespace web {

  /// A Text widget handles putting text on a web page that can be controlled and modified.

  class Text : public internal::WidgetFacet<Text> {
    friend class TextInfo;
  protected:

    class TextInfo : public internal::WidgetInfo {
      friend Text;
    protected:
      DynamicString strings;    ///< All string (and functions returning strings) in Text widget.
      bool append_ok;           ///< Can this Text widget be extended?

      TextInfo(const std::string & in_id="") : internal::WidgetInfo(in_id), append_ok(true) { ; }
      TextInfo(const TextInfo &) = delete;               // No copies of INFO allowed
      TextInfo & operator=(const TextInfo &) = delete;   // No copies of INFO allowed
      virtual ~TextInfo() { ; }

      std::string GetTypeName() const override { return "TextInfo"; }

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
    Text(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsText()); }
    ~Text() { ; }

    using INFO_TYPE = TextInfo;

    /// How many text items are contained?
    size_t GetSize() const { return Info()->strings.GetSize(); }

    /// Erase current text.
    Text & Clear() { Info()->strings.Clear(); return *this; }
  };

  /// Add new text to this string.
  Widget Text::TextInfo::Append(const std::string & text) {
    if (!append_ok) return ForwardAppend(text);  // If text widget cannot append, forward to parent.
    strings.Append(text);                        // Record the new string being added.
    if (state == Widget::ACTIVE) ReplaceHTML();  // If node is active, immediately redraw!
    return web::Text(this);
  }

  /// Add a function that produces text to this widget.  Every time the widget is re-drawn, the
  /// function will be re-run to get the latest version of the text.  When a Live() function
  /// wraps a variable it simply makes sure that this version of Append is called so that the
  /// value of the variable is kept live.
  Widget Text::TextInfo::Append(const std::function<std::string()> & fun) {
    if (!append_ok) return ForwardAppend(fun);   // If text widget cannot append, forward to parent.
    strings.Append(fun);                         // Record the new function being added.
    if (state == Widget::ACTIVE) ReplaceHTML();  // If node is active, immediately redraw!
    return web::Text(this);
  }

}
}

#endif
