/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file Document.hpp
 *  @brief Manage an entire document.
 *
 *  The Document class is built off of Div, but initializes the EMP web framework, if
 *  neeeded, and activates itself.  It also provides quick ways to add and lookup
 *  widgets.
 *
 *  For example, you can use doc.AddButon(...) to add a new button to the document,
 *  where the ... can be any of the mechanisms to build a new button.  This technique
 *  works for any widget type.
 *
 *  You can also look up any widget by name.  For example, if you previously created a
 *  Canvas widget with the HTML id "my_canvas", you can look it up later by using
 *  doc.Canvas("my_canvas")
 */

#ifndef EMP_WEB_DOCUMENT_HPP_INCLUDE
#define EMP_WEB_DOCUMENT_HPP_INCLUDE

#include "events.hpp"

#include "Button.hpp"
#include "Canvas.hpp"
#include "Div.hpp"
#include "Element.hpp"
#include "FileInput.hpp"
#include "Image.hpp"
#include "Input.hpp"
#include "Selector.hpp"
#include "Table.hpp"
#include "TextArea.hpp"
#include "Text.hpp"

#include "canvas_utils.hpp"
#include "color_map.hpp"

// Create an optional alternative to main, emp_main, that is only available after a document
// has loaded and is ready.
#ifdef EMPIRICAL

#define emp_main() emp_main_on_ready();              \
  int main() {                                       \
    emp::web::OnDocumentReady( emp_main_on_ready );  \
  }                                                  \
  emp_main_on_ready()

#else
#define emp_main main
#endif

namespace emp {
namespace web {

  class Document : public web::Div {
  public:
    Document(const std::string & doc_id) : web::Div(doc_id) { Activate(); }
    ~Document() { ; }

    // Retrieve specific types of widgets.

    // Shortcut adders for Widgets
    template <class... T> web::Button AddButton(T &&... args){
      web::Button new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Canvas AddCanvas(T &&... args){
      web::Canvas new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::FileInput AddFileInput(T &&... args){
      web::FileInput new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Image AddImage(T &&... args) {
      web::Image new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Selector AddSelector(T &&... args){
      web::Selector new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Div AddDiv(T &&... args) {
      web::Div new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Table AddTable(T &&... args) {
      web::Table new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Text AddText(T &&... args)  {
      web::Text new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::TextArea AddTextArea(T &&... args)  {
      web::TextArea new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }


    // Setup a quick way to retrieve old widgets by name.
    web::Button Button (const std::string & in_id) { return web::Button(Find(in_id)); }
    web::Canvas Canvas (const std::string & in_id) { return web::Canvas(Find(in_id)); }
    web::Element Element (const std::string & in_id) { return web::Element(Find(in_id)); }
    web::FileInput FileInput (const std::string & in_id) { return web::FileInput(Find(in_id)); }
    web::Input Input (const std::string & in_id) { return web::Input(Find(in_id)); }
    web::Image Image (const std::string & in_id) { return web::Image(Find(in_id)); }
    web::Selector Selector (const std::string & in_id) { return web::Selector(Find(in_id)); }
    web::Div Div (const std::string & in_id) { return web::Div(Find(in_id)); }
    web::Table Table (const std::string & in_id) { return web::Table(Find(in_id)); }
    web::Text Text (const std::string & in_id) { return web::Text(Find(in_id)); }
    web::TextArea TextArea (const std::string & in_id) { return web::TextArea(Find(in_id)); }

  };

}
}


#endif // #ifndef EMP_WEB_DOCUMENT_HPP_INCLUDE
