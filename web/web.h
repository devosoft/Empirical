#ifndef EMP_WEB_H
#define EMP_WEB_H

#include "../emtools/init.h"

#include "Button.h"
#include "Canvas.h"
#include "Image.h"
#include "Selector.h"
#include "Slate.h"
#include "Table.h"
#include "Text.h"


namespace emp {
namespace web {

  bool Initialize() {

    // Make sure we only run Initialize once!
    static bool init = false;
    if (init) return false;
    init = true;

    // Run the base-level initialize in case it hasn't be run yet.
    emp::Initialize();

    // Setup the animation callback in Javascript
    EM_ASM({
        window.requestAnimFrame = (function(callback) {
            return window.requestAnimationFrame
              || window.webkitRequestAnimationFrame
              || window.mozRequestAnimationFrame
              || window.oRequestAnimationFrame
              || window.msRequestAnimationFrame
              || function(callback) { window.setTimeout(callback, 1000 / 60); };
          })();
      });

    return true;
  }


  //  A Document is a root-level slate that automatically initializes Empirical and starts
  //  out in an active state.

  class Document : public web::Slate {
  public:
    Document(const std::string & doc_id) : web::Slate(doc_id) {
      emp::web::Initialize();
      Activate();
    }
    ~Document() { ; }

    // Retrieve specific types of widgets.

    // Shortcut adders for Widgets
    template <class... T> web::Button AddButton(T... args){
      web::Button new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Canvas AddCanvas(T... args){
      web::Canvas new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Image AddImage(T... args) {
      web::Image new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Selector AddSelector(T... args){
      web::Selector new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Slate AddSlate(T... args) {
      web::Slate new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Table AddTable(T... args) {
      web::Table new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }
    template <class... T> web::Text AddText(T... args)  {
      web::Text new_widget(std::forward<T>(args)...);
      info->Append(new_widget);
      return new_widget;
    }

    // Setup a quick way to retrieve old widgets by name.
    web::Button & Button (const std::string & in_id) {
      emp_assert( static_cast<web::Button *>(&(Find(in_id))) );
      return (web::Button &) Find(in_id);
    }
    web::Canvas & Canvas (const std::string & in_id) {
      emp_assert( static_cast<web::Canvas *>(&(Find(in_id))) );
      return (web::Canvas &) Find(in_id);
    }
    web::Image & Image (const std::string & in_id) {
      emp_assert( static_cast<web::Image *>(&(Find(in_id))) );
      return (web::Image &) Find(in_id);
    }
    web::Selector & Selector (const std::string & in_id) {
      emp_assert( static_cast<web::Selector *>(&(Find(in_id))) );
      return (web::Selector &) Find(in_id);
    }
    web::Slate & Slate (const std::string & in_id) {
      emp_assert( static_cast<web::Slate *>(&(Find(in_id))) );
      return (web::Slate &) Find(in_id);
    }
    web::Table & Table (const std::string & in_id) {
      emp_assert( static_cast<web::Table *>(&(Find(in_id))) );
      return (web::Table &) Find(in_id);
    }
    web::Text & Text (const std::string & in_id) {
      emp_assert( static_cast<web::Text *>(&(Find(in_id))) );
      return (web::Text &) Find(in_id);
    }

  };


  // Some helper functions.
  // Live keyword means that whatever is passed in needs to be re-evaluated every update.
  namespace internal {

    // If a variable is passed in to live, construct a function to look up its current value.
    template <typename VAR_TYPE>
    std::function<std::string()> Live_impl(VAR_TYPE & var, bool) {
      return [&var](){ return emp::to_string(var); };
    }
    
    // If anything else is passed in, assume it is a function!
    template <typename IN_TYPE>
    std::function<std::string()> Live_impl(IN_TYPE && fun, int) {
      return [fun](){ return emp::to_string(fun()); };
    }
  }

  template <typename T>
  std::function<std::string()> Live(T && val) {
    return internal::Live_impl(std::forward<T>(val), true);
  }

}
}

#endif
