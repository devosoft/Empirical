//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2020.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef __EMP_D3_SELECTION_H__
#define __EMP_D3_SELECTION_H__

#include "d3_init.h"

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include <array>

#include "../../base/assert.h"
#include "../js_utils.h"
#include "../JSWrap.h"

namespace D3 {

namespace internal {

  /// You probably never want to instantiate this class. Its sole purpose is to hold code for
  /// methods that are common to selections and transitions.
  ///
  /// Developer note: It's also handy if you want to allow a function to accept either a selection
  /// or transition. This is a good idea any time you are only using methods that are applicable to
  /// either, and the person calling the function may want to animate its results.
  template <typename DERIVED>
  class SelectionOrTransition : public D3_Base {
  public:

      SelectionOrTransition() { ; }
      SelectionOrTransition(int id) : D3_Base(id) { ; }
      SelectionOrTransition(const SelectionOrTransition<DERIVED> & s) : D3_Base(s) { ; }

      /// Create a new selection/transition containing the first element matching the
      /// [selector] string that are within this current selection/transition
      DERIVED Select(const std::string & selector) const {
        const int new_id = NextD3ID();

        EM_ASM({
          const id = $0;
          const selector = UTF8ToString($1);
          const new_id = $2;
          var new_selection = emp_d3.objects[id].select(selector);
          emp_d3.objects[new_id] = new_selection;
        }, this->id, selector.c_str(), new_id);

        return DERIVED(new_id);
      }

      /// Create a new selection/transition containing all elements matching the [selector]
      /// string that are within this current selection/transition
      DERIVED SelectAll(const std::string & selector) const {
        const int new_id = NextD3ID();

        EM_ASM({
          const id = $0;
          const selector = UTF8ToString($1);
          const new_id = $2;
  	      var new_selection = emp_d3.objects[id].selectAll(selector);
          emp_d3.objects[new_id] = new_selection;
        }, this->id, selector.c_str(), new_id);

        return DERIVED(new_id);
      }

      // TODO - Consider re-implementing the 'Call' wrapper but allow for arbitrary arguments to the
      //        function being called.

      /// Returns a new selection/transition, representing the current selection/transition
      /// filtered by [selector]. [selector] can be a C++ function that returns a bool, a
      /// string representing a function in either the d3, emp, or window namespaces that
      /// returns a bool, or a string containing a selector to filter by.
      ///
      /// For more information see the
      /// [D3 documentation](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md#filter)
      DERIVED Filter(const std::string & selector) const {
        const int new_id = NextD3ID();

        EM_ASM({
          const id = $0;
          const new_id = $1;
          const selector_str = UTF8ToString($2);
          var sel = emp_d3.find_function(selector_str);
          emp_d3.objects[new_id] = emp_d3.objects[id].filter(sel);
        }, this->id, new_id, selector.c_str());

        return DERIVED(new_id);
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED, decltype(&T::operator())>
      Filter(T selector) const {
        const int new_id = NextD3ID();
        D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(filter, selector);
        StoreNewObject(new_id);
        return DERIVED(new_id);
      }

      /// @endcond


      /// Call the given function on each element of the selection/transition. [function] can
      /// either be a C++ function or a string with the name of a Javascript function in the d3,
      /// emp, or current window namespace.
      DERIVED & Each(const std::string & function_name) {

        EM_ASM({
          const id = $0;
          const func_name_str = UTF8ToString($1);
          var sel = emp_d3.find_function(func_name_str);
          emp_d3.objects[new_id] = emp_d3.objects[id].each(sel);
        }, this->id, function_name.c_str());

        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      Each(T function) {
        D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(each, function);
        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      /// Remove the elements in this selection/transition from the document
      /// For transitions, this happens at the end of the transition.
      void Remove() {
        EM_ASM({
          const id = $0;
          emp_d3.objects[id].remove();
        }, this->id);
      }

      DERIVED Merge(DERIVED & other) {
        const int new_id = NextD3ID();
        EM_ASM({
          const id = $0;
          const other_id = $1;
          const new_id = $2;
          emp_d3.objects[new_id] = emp_d3.objects[id].merge(emp_d3.objects[other_id]);
        }, this->id, other.GetID(), new_id);
        return DERIVED(new_id);
      }


      /**************************************************************************//**
      * @name Setters
      *
      * There are three main types of values you might want to change about a selection:
      * attributes (use `SetAttr`), styles (use `SetStyle`), and properties (use `SetProperty`).
      * The distinction between these types is rooted in how they are represented in web languages
      * (Javascript, CSS, and HTML) and would ideally be abstracted in this wrapper but can't be.
      *
      * Additional traits you can set include text and html.
      *
      * Advanced note: In D3.js, the same functions are used to set and get values (depending on
      * whether an argument is passed). Because C++ needs to have clearly defined
      * return types we need separate getters for each return type.
      *
      * @{
      ***********************************************/

      /** Assigns [value] to the selection's [name] attribute. Value can be any primitive
      type, a string, a function object, or a lambda. If a string is passed, it can be a normal string, or
      the name of a function in d3, emp (such as one created with JSWrap), or the local window.
      If it is a function name, that function will be run, receiving bound data, if any, as input*/

      /* Version for strings
      This will break if someone happens to use a string that
      is identical to a function name
      */

      DERIVED & SetAttr(const std::string & name, const std::string & value) {

        EM_ASM({
          const id = $0;
          const func_name_str = UTF8ToString($1);
          const attr_name_str = UTF8ToString($2);
          const value = emp_d3.find_function(func_name_str);
          emp_d3.objects[new_id] = emp_d3.objects[id].attr(attr_name_str, value);
        }, this->id, value.c_str(), name.c_str());

        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      // This version handles values that are not functions or strings
      // is_fundamental safely excludes lambdas
      template <typename T>
      typename std::enable_if<std::is_fundamental<T>::value, DERIVED&>::type
      SetAttr(const std::string & name, T value) {

        EM_ASM({
          emp_d3.objects[$0].attr(UTF8ToString($1), $2)
          }, this->id, name.c_str(), value);
        return *(static_cast<DERIVED *>(this));
      }

      // This version handles values that are C++ functions and wraps them with JSWrap
      // If a function is being used repeatedly, it may be more efficient to wrap it
      // once and then pass the name as a string.
      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      SetAttr(const std::string & name, T value) {

        // This should probably be JSWrapOnce, but that breaks the visualization
        uint32_t fun_id = emp::JSWrap(value, "", false);

        EM_ASM({
          emp_d3.objects[$0].attr(UTF8ToString($1), function(d, i, k) {
                                                        return emp.Callback($2, d, i, k);
                                                      });
        }, this->id, name.c_str(), fun_id);

        emp::JSDelete(fun_id);

        return *(static_cast<DERIVED *>(this));
      }

      /* We also need a const char * version, because the template version will be a
      better match for raw strings than the std::string version
      */
      DERIVED& SetAttr(const std::string & name, const char * value) {

        EM_ASM({
          const id = $0;
          const func_name_str = UTF8ToString($1);
          const attr_name_str = UTF8ToString($2);
          const value = emp_d3.find_function(func_name_str);
          emp_d3.objects[new_id] = emp_d3.objects[id].attr(attr_name_str, value);
        }, this->id, value, name.c_str());

        return *(static_cast<DERIVED *>(this));
      }

      /* Version for containers */
      template <typename T>
      typename std::enable_if<T::value_type != "", DERIVED&>::type
      SetAttr(const std::string & name, T value) {
        emp::pass_array_to_javascript(value); // This works for all containers that store data contiguously.

        EM_ASM({
    	    emp_d3.objects[$0].attr(UTF8ToString($1), emp_i.__incoming_array);
        }, this->id, name.c_str());

        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      /** Sets the selection's [name] style to [value]. This is the same idea as
      SetAttr, except for CSS styles. Value can be any primitive
      type, a string, a function object, or a lambda. If a string is passed, it can be a normal string, or
      the name of a function in d3, emp (such as one created with JSWrap), or the local window.
      If it is a function name, that function will be run, receiving bound data, if any, as input

      There is a third optional argument, a boolean indicating whether you want to give
      this setting priority.
      */

      //std::string version because std::strings are better
      DERIVED& SetStyle(const std::string & name, const std::string & value, bool priority=false) {

        EM_ASM({
          const id = $0;
          const style_name = UTF8ToString($1);
          const func_string = UTF8ToString($2);
          const priority = ($3 != 0);
          const func_value = emp_d3.find_function(func_string);
          if (priority) {
            emp_d3.objects[id].style(style_name, func_value, "important");
          } else {
            emp_d3.objects[id].style(style_name, func_value);
          }
        }, this->id, name.c_str(), value.c_str(), (int32_t)priority);

        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      // Const char * version so the generic template doesn't get greedy with
      // string literals
      DERIVED& SetStyle(const std::string & name, const char * value, bool priority=false) {

        EM_ASM({
          const id = $0;
          const style_name = UTF8ToString($1);
          const func_string = UTF8ToString($2);
          const priority = ($3 != 0);
          const func_value = emp_d3.find_function(func_string);
          if (priority) {
            emp_d3.objects[id].style(style_name, func_value, "important");
          } else {
            emp_d3.objects[id].style(style_name, func_value);
          }
        }, this->id, name.c_str(), value, (int32_t)priority);

        return *(static_cast<DERIVED *>(this));
      }

      // This version handles values that are C++ functions and wraps them with JSWrap
      // If a function is being used repeatedly, it may be more efficient to wrap it
      // once and then pass the name as a string.
      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      SetStyle(const std::string & name, T value) {
        D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(style, name.c_str(), value);
        return *(static_cast<DERIVED *>(this));
      }

      // Generic template version
      template <typename T>
      typename std::enable_if<std::is_fundamental<T>::value, DERIVED&>::type
      SetStyle(const std::string & name, T value, bool priority=false) {
        if (priority) {
    	    EM_ASM({
            emp_d3.objects[$0].style(UTF8ToString($1), $2, "important")
          }, this->id, name.c_str(), value);
        } else {
    	    EM_ASM({
            emp_d3.objects[$0].style(UTF8ToString($1), $2)
          }, this->id, name.c_str(), value);
        }
        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      /// Sets this selection's text to the specified string, or the string returned by running the
      /// specified function on the element's bound data
      DERIVED & SetText(const std::string & text) {
        EM_ASM({
          const id = $0;
          const new_id = $1;
          const value_name = UTF8ToString($2);
          const value = emp_d3.find_function(value_name);
          emp_d3.objects[new_id] = emp_d3.objects[id].text(value);
        }, this->id, new_id, text.c_str());

        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      SetText(T func) {
        D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(text, func);
        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      // This stuff isn't implemented the same for selections and transitions
      // but we want it to all show up together in the docs
      #ifdef DOXYGEN_RUNNING

      /// Sets special properties of DOM elements (e.g. "checked" for checkboxes)
      /// Value can be a number, function, string, or string naming a Javascript function
      /// See the [d3 documentation](https://github.com/d3/d3-selection#selection_property)
      /// for more information.
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      // std::string version
      DERIVED & SetProperty(const std::string & name, const std::string & value) { return *(static_cast<DERIVED *>(this)); }

      /// Sets this selection's inner html to the specified string, or the string returned by running the
      /// specified function on the element's bound data
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      DERIVED & SetHtml(const std::string & value) { return *(static_cast<DERIVED *>(this)); }

      /// Change whether or not element in this selection have the [classname] class.
      /// Example: Add the data-point class with selection.SetClassed("data-point", true);
      /// Value can also be a function that takes bound data and returns a bool
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      DERIVED & SetClassed(const std::string & classname, bool value) { return *(static_cast<DERIVED *>(this)); }

      #endif

      /// @}

      /**************************************************************************//**
      * @name Getters
      *
      * There are three main types of values you might want to access about a selection:
      * attributes (use `GetAttr`), styles (use `GetStyle`), and properties (use `GetProperty`).
      * The distinction between these types is rooted in how they are represented in web languages
      * (Javascript, CSS, and HTML) and would ideally be abstracted in this wrapper but can't be.
      *
      * Additional traits you can set include text and html.
      *
      * Advanced note: In D3.js, the same functions are used to set and get values (depending on
      * whether an argument is passed). Because C++ needs to have clearly defined
      * return types (and because different macros are required to return different types from
      * Javascript), we need separate getters for each return type.
      *
      * @{
      ***********************************************/

      /// Get the value of this object's [name] attribute when it's a string
      std::string GetAttrString(const std::string & name) const {
        EM_ASM({
  	      var text = emp_d3.objects[$0].attr(UTF8ToString($1));
  	      emp.PassStringToCpp(text);
        }, this->id, name.c_str());
        return emp::pass_str_to_cpp();
      }

      /// Get the value of this object's [name] attribute when it's an int
      int GetAttrInt(const std::string & name) const {
        return EM_ASM_INT({
  	      return emp_d3.objects[$0].attr(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get the value of this object's [name] attribute when it's a double
      double GetAttrDouble(const std::string & name) const {
        return EM_ASM_DOUBLE({
  	      return emp_d3.objects[$0].attr(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get the value of this object's [name] style when it's a string
      std::string GetStyleString(const std::string & name) const {
        EM_ASM({
  	      var style = emp_d3.objects[$0].style(UTF8ToString($1));
  	      emp.PassStringToCpp(style);
        }, this->id, name.c_str());
        return emp::pass_str_to_cpp();
      }

      /// Get the value of this object's [name] style when it's an int
      int GetStyleInt(const std::string & name) const {
        return EM_ASM_INT({
  	      return emp_d3.objects[$0].style(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get the value of this object's [name] style when it's a double
      double GetStyleDouble(const std::string & name) const {
        return EM_ASM_DOUBLE({
  	      return emp_d3.objects[$0].style(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get this object's text
      std::string GetText() const {
        EM_ASM({
          var text = emp_d3.objects[$0].text();
  	      emp.PassStringToCpp(text);
        }, this->id);
        return emp::pass_str_to_cpp();
      }

      // GetHtml and GetProperty are implemented differently for transitions and selections
      // These are placeholders for doxygen

      #ifdef DOXYGEN_RUNNING

      /// Get this object's html
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      std::string GetHtml() {return "";}

      /// Get the value of this object's [name] property when its a string
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      std::string GetPropertyString(const std::string & name) {return "";}

      /// Get the value of this object's [name] property when it's an int
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      int GetPropertyInt(const std::string & name) {return 0;}

      /// Get the value of this object's [name] property when it's a double
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      double GetPropertyDouble(const std::string & name) {return 0;}

      #endif

      /// Returns true if there are no elements in this selection (or all elements are null)
      bool Empty() const {
        const int empty = EM_ASM_INT({return Number(emp_d3.objects[$0].empty())},
             this->id);
        return (bool)empty;
      }

      /// Returns number of elements in this selection
      int Size() const {
        return EM_ASM_INT({return emp_d3.objects[$0].size()},
             this->id);
      }

    /// @}
  };
}

}

#endif
