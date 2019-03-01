//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef __SELECTION_H__
#define __SELECTION_H__

#include "d3_init.h"
#include "utils.h"
#include "dataset.h"

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include <array>

#include "../../base/assert.h"
#include "../js_utils.h"
#include "../JSWrap.h"

/// @cond EMSCRIPTEN_STUFF
extern "C" {
  extern int n_objects();
}
/// @endcond

namespace D3 {

  class Dataset;

  /// You probably never want to instantiate this class. Its sole purpose is to hold code for
  /// methods that are common to selections and transitions.
  ///
  /// Developer note: It's also handy if you want to allow a function to accept either a selection
  /// or transition. This is a good idea any time you are only using methods that are applicable to
  /// either, and the person calling the function may want to animate its results.
  template <typename DERIVED>
  class SelectionOrTransition : public D3_Base {
  public:
      SelectionOrTransition(){;};
      SelectionOrTransition(int id) : D3_Base(id){;};
      SelectionOrTransition(const SelectionOrTransition<DERIVED> & s) : D3_Base(s){;};

      /// Create a new selection/transition containing the first element matching the
      /// [selector] string that are within this current selection/transition
      DERIVED Select(std::string selector) const {
        int new_id = NextD3ID();

        EM_ASM_ARGS({
  	      var new_selection = js.objects[$0].select(UTF8ToString($1));
  	      js.objects[$2] = new_selection;
        }, this->id, selector.c_str(), new_id);

        return DERIVED(new_id);
      }

      /// Create a new selection/transition containing all elements matching the [selector]
      /// string that are within this current selection/transition
      DERIVED SelectAll(std::string selector) const {
        int new_id = NextD3ID();

        EM_ASM_ARGS({
        //   console.log($0, js.objects[$0]);
  	      var new_selection = js.objects[$0].selectAll(UTF8ToString($1));
          js.objects[$2] = new_selection;
        }, this->id, selector.c_str(), new_id);

        return DERIVED(new_id);
      }

      /// Call the given function once on the entire selection/transition. [function] can either
      /// be a C++ function or a string with the name of a Javascript function in the d3, emp, or
      /// current window namespace.
      /// To get around the problem of passing selections into C++, this function assumes that the
      /// function you are passing expects a single argument: an int, representing the id of the
      /// selection to be operated on (which you can then convert to a selection object with
      /// `D3::Selection(i)`).
      // TODO: Allow arguments
      DERIVED& Call(std::string function){
        EM_ASM_ARGS({
          var func_string = UTF8ToString($1);
          if (typeof window[func_string] === "function") {
            func_string = window[func_string];
          } else if (typeof window["emp"][func_string] === "function") {
            func_string = window["emp"][func_string];
          } else if (typeof window["d3"][func_string] === "function") {
            func_string = window["d3"][func_string];
          }

          emp.__new_object = js.objects[$0].call(function(sel){return func_string($0);});
        }, this->id, function.c_str());

        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      Call(T function){
        uint32_t fun_id = emp::JSWrap(function, "", false);
        EM_ASM_ARGS({
          emp.__new_object = js.objects[$0].call(function(selection) {
                                                  return emp.Callback($1, $0);
                                                 });
        }, this->id, fun_id);
        emp::JSDelete(fun_id);

        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      /// Returns a new selection/transition, representing the current selection/transition
      /// filtered by [selector]. [selector] can be a C++ function that returns a bool, a
      /// string representing a function in either the d3, emp, or window namespaces that
      /// returns a bool, or a string containing a selector to filter by.
      ///
      /// For more information see the
      /// [D3 documentation](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md#filter)
      DERIVED Filter(std::string selector) const {

        int new_id = NextD3ID();
        D3_CALLBACK_METHOD_1_ARG(filter, selector.c_str())
        StoreNewObject(new_id);
        return DERIVED(new_id);
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED, decltype(&T::operator())>
      Filter(T selector) const {

        int new_id = NextD3ID();
        D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(filter, selector)
        StoreNewObject(new_id);
        return DERIVED(new_id);
      }

      /// @endcond

      /// Call the given function on each element of the selection/transition. [function] can
      /// either be a C++ function or a string with the name of a Javascript function in the d3,
      /// emp, or current window namespace.

      DERIVED& Each(std::string function){
        D3_CALLBACK_METHOD_1_ARG(each, function.c_str())
        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      Each(T function){
        D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(each, function)
        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      /// Remove the elements in this selection/transition from the document
      /// For transitions, this happens at the end of the transition.
      void Remove(){
        EM_ASM_ARGS({js.objects[$0].remove()},
          this->id);
      }

      DERIVED Merge(DERIVED & other) {
          int new_id = NextD3ID();
        //   std::cout << "New id should be: " << new_id << std::endl;
          EM_ASM_ARGS({
              js.objects[$2] = js.objects[$0].merge(js.objects[$1]);
            //   console.log("Merged: ", js.objects[$2], $2);
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

      DERIVED& SetAttr(std::string name, std::string value) {

        D3_CALLBACK_METHOD_2_ARGS(attr, name.c_str(), value.c_str())
        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      //This version handles values that are not functions or strings
      //is_fundamental safely excludes lambdas
      template <typename T>
      typename std::enable_if<std::is_fundamental<T>::value, DERIVED&>::type
      SetAttr(std::string name, T value) {

        EM_ASM_ARGS({js.objects[$0].attr(UTF8ToString($1), $2)},
    		  this->id, name.c_str(), value);
        return *(static_cast<DERIVED *>(this));
      }

      //This version handles values that are C++ functions and wraps them with JSWrap
      //If a function is being used repeatedly, it may be more efficient to wrap it
      //once and then pass the name as a string.
      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      SetAttr(std::string name, T value) {

        //This should probably be JSWrapOnce, but that breaks the visualization
        uint32_t fun_id = emp::JSWrap(value, "", false);

        EM_ASM_ARGS({
          js.objects[$0].attr(UTF8ToString($1), function(d, i, k) {
                                                        return emp.Callback($2, d, i, k);
                                                      });
        }, this->id, name.c_str(), fun_id);

        emp::JSDelete(fun_id);

        return *(static_cast<DERIVED *>(this));
      }

      /* We also need a const char * version, because the template version will be a
      better match for raw strings than the std::string version
      */

      DERIVED& SetAttr(std::string name, const char * value) {

        D3_CALLBACK_METHOD_2_ARGS(attr, name.c_str(), value)
        return *(static_cast<DERIVED *>(this));
      }

      /* Version for containers */
      template <typename T>
      typename std::enable_if<T::value_type != "", DERIVED&>::type
      SetAttr(std::string name, T value) {
        emp::pass_array_to_javascript(value);

        EM_ASM_ARGS({
    	    js.objects[$0].attr(UTF8ToString($1), emp_i.__incoming_array);
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
      DERIVED& SetStyle(std::string name, std::string value, bool priority=false){
        if (priority){
    	    EM_ASM_ARGS({
            var func_string = UTF8ToString($2);
    	    if (typeof window[func_string] === "function") {
              func_string = window[func_string];
            }
            for (name in {d3:"d3", emp:"emp"}) {
              if (typeof window[name][func_string] === "function") {
                func_string = window[name][func_string];
              }
            }
    	    js.objects[$0].style(UTF8ToString($1), in_string, "important");
    	    }, this->id, name.c_str(), value.c_str());
        } else {
    	    D3_CALLBACK_METHOD_2_ARGS(style, name.c_str(), value.c_str())
        }
        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      //Const char * version so the generic template doesn't get greedy with
      //string literals
      DERIVED& SetStyle(std::string name, const char* value, bool priority=false){
        if (priority){
    	    EM_ASM_ARGS({
    	      var func_string = UTF8ToString($2);
    	      if (typeof window[func_string] === "function") {
                func_string = window[func_string];
              }
              for (name in {d3:"d3", emp:"emp"}) {
                if (typeof window[name][func_string] === "function") {
                  func_string = window[name][func_string];
                }
              };
    	      js.objects[$0].style(UTF8ToString($1), in_string, "important");
    	    }, this->id, name.c_str(), value);
        } else {
    	    D3_CALLBACK_METHOD_2_ARGS(style, name.c_str(), value)
        }
        return *(static_cast<DERIVED *>(this));
      }

      //This version handles values that are C++ functions and wraps them with JSWrap
      //If a function is being used repeatedly, it may be more efficient to wrap it
      //once and then pass the name as a string.
      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      SetStyle(std::string name, T value) {
        D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(style, name.c_str(), value);
        return *(static_cast<DERIVED *>(this));
      }

      //Generic template version
      template <typename T>
      typename std::enable_if<std::is_fundamental<T>::value, DERIVED&>::type
      SetStyle(std::string name, T value, bool priority=false){
        if (priority){
    	    EM_ASM_ARGS({js.objects[$0].style(UTF8ToString($1), $2, "important")},
              this->id, name.c_str(), value);
          }
        else {
    	    EM_ASM_ARGS({js.objects[$0].style(UTF8ToString($1), $2)},
              this->id, name.c_str(), value);
        }
        return *(static_cast<DERIVED *>(this));
      }

      /// @endcond

      /// Sets this selection's text to the specified string, or the string returned by running the
      /// specified function on the element's bound data
      DERIVED& SetText(std::string text){
        D3_CALLBACK_METHOD_1_ARG(text, text.c_str())
        return *(static_cast<DERIVED *>(this));
      }

      /// @cond TEMPLATES

      template <typename T>
      emp::sfinae_decoy<DERIVED&, decltype(&T::operator())>
      SetText(T func){
        D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(text, func)
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
      // std::string verison
      DERIVED& SetProperty(std::string name, std::string value) {return *(static_cast<DERIVED *>(this));}

      /// Sets this selection's inner html to the specified string, or the string returned by running the
      /// specified function on the element's bound data
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      DERIVED& SetHtml(std::string value) {return *(static_cast<DERIVED *>(this));}

      /// Change whether or not element in this selection have the [classname] class.
      /// Example: Add the data-point class with selection.SetClassed("data-point", true);
      // Value can also be a function that takes bound data and returns a bool
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      DERIVED& SetClassed(std::string classname, bool value) {return *(static_cast<DERIVED *>(this));}

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
      std::string GetAttrString(std::string name) const {
        char * buffer = (char *)EM_ASM_INT({
  	      var text = js.objects[$0].attr(UTF8ToString($1));
  	      var buffer = Module._malloc(text.length+1);
  	      Module.writeStringToMemory(text, buffer);
  	      return buffer;
        }, this->id, name.c_str());

        std::string result = std::string(buffer);
        free(buffer);
        return result;
      }

      /// Get the value of this object's [name] attribute when it's an int
      int GetAttrInt(std::string name) const {
        return EM_ASM_INT({
  	      return js.objects[$0].attr(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get the value of this object's [name] attribute when it's a double
      double GetAttrDouble(std::string name) const {
        return EM_ASM_DOUBLE({
  	      return js.objects[$0].attr(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get the value of this object's [name] style when it's a string
      std::string GetStyleString(std::string name) const {
        char * buffer = (char *)EM_ASM_INT({
  	      var text = js.objects[$0].style(UTF8ToString($1));
  	      var buffer = Module._malloc(text.length+1);
  	      Module.writeStringToMemory(text, buffer);
  	      return buffer;
        }, this->id, name.c_str());

        std::string result = std::string(buffer);
        free(buffer);
        return result;
      }

      /// Get the value of this object's [name] style when it's an int
      int GetStyleInt(std::string name) const {
        return EM_ASM_INT({
  	      return js.objects[$0].style(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get the value of this object's [name] style when it's a double
      double GetStyleDouble(std::string name) const {
        return EM_ASM_DOUBLE({
  	      return js.objects[$0].style(UTF8ToString($1));
        }, this->id, name.c_str());
      }

      /// Get this object's text
      std::string GetText() const {

        char * buffer = (char *)EM_ASM_INT({
  	      var text = js.objects[$0].text();
  	      var buffer = Module._malloc(text.length+1);
  	      Module.writeStringToMemory(text, buffer);
  	      return buffer;
  	    }, this->id);

        std::string result = std::string(buffer);
        free(buffer);
        return result;
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
      std::string GetPropertyString(std::string name) {return "";}

      /// Get the value of this object's [name] property when it's an int
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      int GetPropertyInt(std::string name) {return 0;}

      /// Get the value of this object's [name] property when it's a double
      ///
      /// Advanced note: This is implemented differently for selection vs transitions. As such,
      /// calling it on a SelectionOrTransition object directly is not supported.
      double GetPropertyDouble(std::string name) {return 0;}

      #endif

      /// Returns true if there are no elements in this selection (or all elements are null)
      bool Empty() const {
        int empty = EM_ASM_INT({return Number(js.objects[$0].empty())},
             this->id);
        return (bool)empty;
      }

      /// Returns number of elements in this selection
      int Size() const {
        return EM_ASM_INT({return js.objects[$0].size()},
             this->id);
      }

    /// @}

  };

  /// [Transitions](https://github.com/d3/d3-3.x-api-reference/blob/master/Transitions.md/)
  /// are similar to selections, but when you make a change to them (attr or style), it will be
  /// animated. For additional discussion of transitions in d3, see
  /// [this article](https://bost.ocks.org/mike/transition/).
  class Transition : public SelectionOrTransition<Transition> {
  public:

    /**************************************************************************//**
    * @name Constructors
    *
    * Usually transitions are constructed from selections by calling the selection.MakeTransition()
    * method. In rare cases you may want to construct a new transition, though.
    *
    * @{
    ***********************************************/

    /// Default constructor - construct empty transition
    Transition(){;};

    /// Advanced: Construct new transition pointing to the [id]th element in js.objects.
    Transition(int id) : SelectionOrTransition(id) {;};

    /// Create a transition from the current transition. If a [name] is specified
    /// the transition will be given that name
    ///
    /// Note: In D3.js this method is just called transition(), but in C++ that would cause a
    /// collision with the constructor
    //TODO: D3 supports passing a selection, but it's kind of a weird edge case
    Transition NewTransition(std::string name="") const {
      int new_id = NextD3ID();
      EM_ASM_ARGS({
 	    var transition = js.objects[$0].transition(UTF8ToString($1));
	    js.objects[$2] = transition;
    }, this->id, name.c_str(), new_id);

      return D3::Transition(new_id);
    }

    #ifndef DOXYGEN_RUNNING

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

    Transition& On(std::string type, std::string listener="null", bool capture=false){

      // Check that the listener is valid
      emp_assert(EM_ASM_INT({
        var func_string = UTF8ToString($0);
        if (func_string == "null") {
          return true;
        }
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        return (typeof func_string === "function");
      }, listener.c_str()) \
      && "String passed to On is not s Javascript function or null", listener);

      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var func_string = UTF8ToString($2);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }

	    if (typeof func_string === "function") {
	      js.objects[$0].on(UTF8ToString($1),
		  func_string,$3);
	    } else {
	      js.objects[$0].on(UTF8ToString($1), null);
	    }

      }, this->id, type.c_str(), listener.c_str(), capture, new_id);

      return (*this);
    }

    /// @cond TEMPLATES

    /// Version for C++ functions
    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    On(std::string type, T listener, bool capture=false){

      uint32_t fun_id = emp::JSWrap(listener, "", false);
      int new_id = NextD3ID();

      EM_ASM_ARGS({
	      js.objects[$0].on(UTF8ToString($1),
		  function(d, i){
		     js.objects[$4] = d3.select(this);
		     emp.Callback($2, d, i, $4);}, $3);
      }, this->id, type.c_str(), fun_id, capture, new_id);

      emp::JSDelete(fun_id);
      return (*this);
    }

    /// @endcond

    Transition& SetDuration(double time) {
        EM_ASM_ARGS({
            js.objects[$0].duration($1);
        }, this->id, time);
        return (*this);
    }

    /// Sets special properties of DOM elements (e.g. "checked" for checkboxes)
    /// Value can be a number, function, string, or string naming a Javascript function
    /// See the [d3 documentation](https://github.com/d3/d3-selection#selection_property)
    /// for more information.
    // std::string verison
    Transition& SetProperty(std::string name, std::string value){
      EM_ASM_ARGS({
        var arg1 = UTF8ToString($1);				                              	\
        var func_string = UTF8ToString($2);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        js.objects[$0].each("end", function(){
          d3.select(this).property(arg1, func_string);
        });
      }, name.c_str(), value.c_str());
      return *this;
    }

    /// @cond TEMPLATES

    // Const char * version so raw strings work
    Transition& SetProperty(std::string name, const char* value){
      EM_ASM_ARGS({
        var arg1 = UTF8ToString($1);				                              	\
        var func_string = UTF8ToString($2);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        js.objects[$0].each("end", function(){
          d3.select(this).property(arg1, func_string);
        });
      }, name.c_str(), value);
      return *this;
    }

    //Generic template version
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, Transition&>::type
    SetProperty(std::string name, T value){
      EM_ASM_ARGS({
          js.objects[$0].each("end", function() {
            d3.select(this).property(UTF8ToString($1), $2);
          });
      }, this->id, name.c_str());
      return *this;
    }

    //This version handles values that are C++ functions and wraps them with JSWrap
    //If a function is being used repeatedly, it may be more efficient to wrap it
    //once and then pass the name as a string.
    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    SetProperty(std::string name, T value) {
      uint32_t fun_id = emp::JSWrap(value, "", false);
      EM_ASM_ARGS({
        js.objects[$0].each("end", function(){
                d3.select(this).property(UTF8ToString($1),
                                          function(d, i, j) {
                                            return emp.Callback($2, d, i, j);
                                        });
        });
      }, this->id, name.c_str(), fun_id);
      emp::JSDelete(fun_id);
      return *this;
    }

    /// @endcond

    /// Sets this selection's inner html to the specified string, or the string returned by running the
    /// specified function on the element's bound data
    Transition& SetHtml(std::string value){
      EM_ASM_ARGS({
        var func_string = UTF8ToString($1);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        js.objects[$0].each("end", function(){
          d3.select(this).html(func_string);
        });
      }, this->id, value.c_str());
      return *this;
    }

    /// @cond TEMPLATES

    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    SetHtml(T func){
      uint32_t fun_id = emp::JSWrap(func, "", false);
      EM_ASM_ARGS({
        js.objects[$0].each("end", function(){
                  d3.select(this).html(function(d, i, j) {
                        return emp.Callback($1, d, i, j);
                  });
        });
      }, this->id, fun_id);
      emp::JSDelete(fun_id);
      return *this;
    }

    /// @endcond

    /// Change whether or not element in this selection have the [classname] class.
    /// Example: Add the data-point class with selection.SetClassed("data-point", true);
    // Value can also be a function that takes bound data and returns a bool
    Transition& SetClassed(std::string classname, bool value) {
      EM_ASM_ARGS({
        js.objects[$0].each("end", function(){
            d3.select(this).classed(UTF8ToString($1), $2);
        });
      }, this->id, classname.c_str(), value);
      return *this;
    }

    ///@cond TEMPLATES

    // Version for C++ function
    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    SetClassed(std::string, std::string classname, T func){
      uint32_t fun_id = emp::JSWrap(func, "", false);
      EM_ASM_ARGS({
        js.objects[$0].each("end", function(){
                  d3.select(this).classed(UTF8ToString($1),
                                            function(d, i, j) {
                                              return emp.Callback($2, d, i, j);
                                          });
        });
      }, this->id, classname.c_str(), fun_id);
      emp::JSDelete(fun_id);

      return *this;
    }

    // Version that allows strings containing function names but warns on other strings
    Transition& SetClassed(std::string classname, std::string value){
      emp_assert(EM_ASM_INT({
        var func_string = UTF8ToString($0);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        return (typeof func_string === "function");
      }, value.c_str()) && "String passed to SetClassed is not a Javascript function", value);

      EM_ASM_ARGS({
        var arg1 = UTF8ToString($1);				                              	\
        var func_string = UTF8ToString($2);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        js.objects[$0].each("end", function(){
          d3.select(this).classed(arg1, func_string);
        });
      }, classname.c_str(), value.c_str());

      return *this;
    }

    ///@endcond

    /** @} */

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

    /// Get this object's html
    std::string GetHtml() const {
      char * buffer = (char *)EM_ASM_INT({
        var text = d3.select(js.objects[$0]).html();
        var buffer = Module._malloc(text.length+1);
        Module.writeStringToMemory(text, buffer);
        return buffer;
      }, this->id);

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] property when its a string
    std::string GetPropertyString(std::string name) const {
      char * buffer = (char *)EM_ASM_INT({
        var text = d3.select(js.objects[$0]).property(UTF8ToString($1));
        var buffer = Module._malloc(text.length+1);
        Module.writeStringToMemory(text, buffer);
        return buffer;
      }, this->id, name.c_str());

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] property when it's an int
    int GetPropertyInt(std::string name) const {
      return EM_ASM_INT({
        return d3.select(js.objects[$0]).property(UTF8ToString($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] property when it's a double
    double GetPropertyDouble(std::string name) const {
      return EM_ASM_DOUBLE({
        return d3.select(js.objects[$0]).property(UTF8ToString($1));
      }, this->id, name.c_str());
    }

    /// @}

    #endif

  };

  /// [Selections](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md/)
  /// are the primary way that d3 allows you to operate on DOM elements
  /// (i.e. objects on your webpage). A selection is effectively an array of DOM elements
  /// that you can act on at the same time and bind a collection of data to.
  ///
  /// For a deep dive into how selections work in d3, see
  /// [this article](https://bost.ocks.org/mike/selection/).
  class Selection : public SelectionOrTransition<Selection> {

  public:

    /// @name Constructors
    /// You may prefer to use the Select or SelectAll functions for improved
    /// code clarity/consistency with d3.js
    /// @{

    /// Default constructor - constructs empty selection
    Selection(){
        EM_ASM_ARGS({js.objects[$0] = d3.selection();}, this->id);
    };

    /// Create Selection object with a specific id.
    ///
    /// Advanced note: This is useful when creating a Selection object to point to a selection
    // that you already created in Javascript and added to js.objects.
    Selection(int id) : SelectionOrTransition(id){
    };

    Selection(const Selection & s) : SelectionOrTransition(s){
    };

    // Selection& operator= (const Selection & other) {
    //     std::cout << "Calling assingment: " << this->id << " " << other.id << std::endl;
    //     this->id = other.id;
    //     return (*this);
    // }

    /// This is the Selection constructor you usually want to use. It takes a string saying what
    /// to select and a bool saying whether to select all elements matching that string [true] or
    /// just the first [false]
    Selection(std::string selector, bool all = false) {
      if (all){
        EM_ASM_ARGS({
  	      js.objects[$0] = d3.selectAll(UTF8ToString($1));
        }, this->id, selector.c_str());
      }
      else {
        EM_ASM_ARGS({
  	      js.objects[$0] = d3.select(UTF8ToString($1));
        }, this->id, selector.c_str());
      }
    };

    /// Destructor
    ~Selection(){};


    /// @}

    /// @name Binding Data
    /// This group of functions allows you to bind data to the current selection and deal with
    /// new data you have just bound (the enter selection) and data that was previously bound to
    /// to the selection but is not present in the set of data that was most recently bound
    /// (the exit selection)
    ///
    /// The process of binding data to a selection is called a "join" in d3-speak. For more
    /// in-depth explanation, see [this article](https://bost.ocks.org/mike/join/).

    /// @{

    /// Bind data to selection. Accepts any contiguous container (such as an array or
    /// vector) or a D3::Dataset object (which stores the data Javascript). Optionally
    /// also accepts a key function to run on each element to determine which elements are
    /// equivalent (if no key is provided, elements are expected to be in the same order
    /// each time you bind data to this selection). This function can either be a string
    /// with the name of a function in Javascript, or it can be a C++ function pointer,
    /// std::function object, or lambda.

    //Option to pass loaded dataset stored in Javascript without translating to C++
    Selection Data(Dataset & values, std::string key=""){
      int update_id = NextD3ID();

      EM_ASM_ARGS({
        //We could make this slightly prettier with macros, but would
        //add an extra comparison
	    var in_string = UTF8ToString($1);
	    var fn = window["emp"][in_string];
	    if (typeof fn === "function"){
	      var update_sel = js.objects[$0].data(js.objects[$2], fn);
        } else if (typeof window["d3"][in_string] === "function") {
	      var update_sel = js.objects[$0].data(js.objects[$2],
						 window["d3"][in_string]);
	    } else if (typeof window[in_string] === "function") {
	      var update_sel = js.objects[$0].data(js.objects[$2],
						 window[in_string]);
	    } else {
	      var update_sel = js.objects[$0].data(js.objects[$2]);
	    }

	    js.objects[$3] = update_sel;
      },this->id, key.c_str(), values.GetID(), update_id);

      Selection update = Selection(update_id);
      return update;
    }

    ///@cond TEMPLATES

    // Accepts Dataset and C++ function as key
    template<typename T>
    emp::sfinae_decoy<Selection, decltype(&T::operator())>
    Data(Dataset & values, T key){
      int update_id = NextD3ID();
      uint32_t fun_id = emp::JSWrap(key, "", false);

  	  EM_ASM_ARGS({
        var update_sel = js.objects[$0].data(js.objects[$2],
                                                function(d,i) {
                                                  return emp.Callback($1, d, i);
                                            });
	    js.objects[$3] = update_sel;

      }, this->id, fun_id, values.GetID(), update_id);

      emp::JSDelete(fun_id);

      Selection update = Selection(update_id);
      return update;
    }

    // template<typename C>
    // emp::sfinae_decoy<Selection, decltype(C::value_type::n_fields)>
    // Data(C values){
    //     std::cout << "using the right one" << std::endl;
    //   int update_id = NextD3ID();
    //   emp::pass_array_to_javascript(values);
    //
    //   EM_ASM_ARGS({
    //     var update_sel = js.objects[$0].data(emp_i.__incoming_array);
	//     js.objects[$1] = update_sel;
    //
    //   }, this->id, update_id);
    //
    //   Selection update = Selection(update_id);
    //   return update;
    // }


    // Accepts string referring to Javascript function
    template<typename C, class = typename C::value_type>
    // typename std::enable_if<std::is_pod<typename C::value_type>::value, Selection>::type
    Selection Data(C values, std::string key=""){
      int update_id = NextD3ID();


        //   std::cout << "In bind data: " << values[0].x0() << std::endl;
      emp::pass_array_to_javascript(values);

  	  EM_ASM_ARGS({
	    var in_string = UTF8ToString($1);
	    var fn = window["emp"][in_string];
	    if (typeof fn === "function"){
	      var update_sel = js.objects[$0].data(emp_i.__incoming_array, fn);
	    } else if (typeof window["d3"][in_string] === "function") {
	      var update_sel = js.objects[$0].data(emp_i.__incoming_array,
						 window["d3"][in_string]);
	    } else if (typeof window[in_string] === "function") {
	      var update_sel = js.objects[$0].data(emp_i.__incoming_array,
						 window[in_string]);
	    } else {
	      var update_sel = js.objects[$0].data(emp_i.__incoming_array);
	    }

	    js.objects[$2] = update_sel;
      }, this->id, key.c_str(), update_id);

      Selection update = Selection(update_id);
      return update;
    }

    // Accepts C++ function as key
    template<typename C, class = typename C::value_type, typename T>
    emp::sfinae_decoy<Selection, decltype(&T::operator())>
    Data(C values, T key){
      int update_id = NextD3ID();
      emp::pass_array_to_javascript(values);
      uint32_t fun_id = emp::JSWrap(key, "", false);

  	  EM_ASM_ARGS({
        var update_sel = js.objects[$0].data(emp_i.__incoming_array,
                                                function(d,i,k) {
                                                  return emp.Callback($1, d, i, k);
                                            });
	    js.objects[$2] = update_sel;
      }, this->id, fun_id, update_id);

      emp::JSDelete(fun_id);

      Selection update = Selection(update_id);
      return update;
    }

    /// @endcond

    Dataset GetData() const {
        int new_id = NextD3ID();
        EM_ASM_ARGS({
            js.objects[$1] = [js.objects[$0].data()];
        }, this->id, new_id);
        return Dataset(new_id);
    }

    /// This function appends the specified type of nodes to this
    /// selection's enter selection, which merges the enter selection
    /// with the update selection.
    ///
    /// Selection must have an enter selection (i.e. have just had data bound to it).

    Selection EnterAppend(std::string type){

      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var append_selection = js.objects[$0].enter()
                               .append(UTF8ToString($1));
	    js.objects[$2] = append_selection;
      }, this->id, type.c_str(), new_id);

      return Selection(new_id);
    }

    /// Insert elements of type [name] into current enter selection
    ///
    /// For more information, see the D3 documention on
    /// [insert](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md#insert)
    Selection EnterInsert(std::string name, std::string before=NULL){
      int new_id = NextD3ID();

      if (before.c_str()){
	    EM_ASM_ARGS({
	      var new_sel = js.objects[$0].enter().insert(UTF8ToString($1),
						  UTF8ToString($2));
	      js.objects[$3] = new_sel;
        }, this->id, name.c_str(), before.c_str(), new_id);
      } else {
	    EM_ASM_ARGS({
	      var new_sel = js.objects[$0].enter().insert(UTF8ToString($1));
	      js.objects[$2] = new_sel;
        }, this->id, name.c_str(), new_id);
      }

      return Selection(new_id);
    }

    /// Sometimes you want to perform multiple operations on the enter selection. If so, you can
    /// use the Enter() method to get the enter selection, rather than using one of the convenience
    /// functions like EnterAppend().
    ///
    /// Returns a selection object pointing at this selection's enter selection.
    Selection Enter() {

      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var enter_selection = js.objects[$0].enter();
	    js.objects[$1] = enter_selection;
      }, this->id, new_id);

      return Selection(new_id);
    }

    /// Pretty much the only thing you ever want to do with the exit() selection
    /// is remove all of the nodes in it. This function does just that.

    ///Selection must have an exit selection (i.e. have just had data bound to it).
    void ExitRemove(){

      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var exit_selection = js.objects[$0].exit().remove();
	    js.objects[$1] = exit_selection;
      }, this->id, new_id);
    }

    /// Usually the only thing you want to do with the exit selection
    /// is remove its contents, in which case you should use the
    /// ExitRemove method. However, advanced users may want to operate
    /// on the exit selection, which is why this method is provided.
    ///
    /// Returns a selection object pointing at this selection's exit selection.
    Selection Exit(){

      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var exit_selection = js.objects[$0].exit();
	    js.objects[$1] = exit_selection;
      }, this->id, new_id);

      return Selection(new_id);
    }

    /// @}

    #ifndef DOXYGEN_RUNNING

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

    /// Sets special properties of DOM elements (e.g. "checked" for checkboxes)
    /// Value can be a number, function, string, or string naming a Javascript function
    /// See the [d3 documentation](https://github.com/d3/d3-selection#selection_property)
    /// for more information.
    // std::string verison
    Selection& SetProperty(std::string name, std::string value){
      D3_CALLBACK_METHOD_2_ARGS(property, name.c_str(), value.c_str())
      return *this;
    }

    /// @cond TEMPLATES

    // Const char * version so raw strings work
    Selection& SetProperty(std::string name, const char* value){
      D3_CALLBACK_METHOD_2_ARGS(property, name.c_str(), value)
      return *this;
    }

    //Generic template version
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, Selection&>::type
    SetProperty(std::string name, T value){
      EM_ASM_ARGS({js.objects[$0].property(UTF8ToString($1),
					   $2)}, this->id, name.c_str());
      return *this;
    }

    //This version handles values that are C++ functions and wraps them with JSWrap
    //If a function is being used repeatedly, it may be more efficient to wrap it
    //once and then pass the name as a string.
    template <typename T>
    emp::sfinae_decoy<Selection&, decltype(&T::operator())>
    SetProperty(std::string name, T value) {
      D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(property, name.c_str(), value);
      return *this;
    }

    /// @endcond

    /// Sets this selection's inner html to the specified string, or the string returned by running the
    /// specified function on the element's bound data
    Selection& SetHtml(std::string value){
      D3_CALLBACK_METHOD_1_ARG(html, value.c_str())
      return *this;
    }

    /// @cond TEMPLATES

    template <typename T>
    emp::sfinae_decoy<Selection&, decltype(&T::operator())>
    SetHtml(T func){
      D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(html, func)
      return *this;
    }

    /// @endcond

    /// Change whether or not element in this selection have the [classname] class.
    /// Example: Add the data-point class with selection.SetClassed("data-point", true);
    // Value can also be a function that takes bound data and returns a bool
    Selection& SetClassed(std::string classname, bool value) {
      EM_ASM_ARGS({
        js.objects[$0].classed(UTF8ToString($1), $2);
      }, this->id, classname.c_str(), value);
      return *this;
    }

    ///@cond TEMPLATES

    // Version for C++ function
    template <typename T>
    emp::sfinae_decoy<Selection&, decltype(&T::operator())>
    SetClassed(std::string, std::string classname, T func){
      D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(classed, classname.c_str(), func)
      return *this;
    }

    // Version that allows strings containing function names but warns on other strings
    Selection& SetClassed(std::string classname, std::string value){
      emp_assert(EM_ASM_INT({
        var func_string = UTF8ToString($0);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        return (typeof func_string === "function");
      }, value.c_str()) && "String passed to SetClassed is not a Javascript function", value);
      D3_CALLBACK_METHOD_2_ARGS(classed, classname.c_str(), value.c_str())
      return *this;
    }

    ///@endcond

    /** @} */

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

    /// Get this object's html
    std::string GetHtml() const {
      char * buffer = (char *)EM_ASM_INT({
	    var text = js.objects[$0].html();
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
	  }, this->id);

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] property when its a string
    std::string GetPropertyString(std::string name) const {
      char * buffer = (char *)EM_ASM_INT({
	    var text = js.objects[$0].property(UTF8ToString($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
	  }, this->id, name.c_str());

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] property when it's an int
    int GetPropertyInt(std::string name) const {
      return EM_ASM_INT({
	    return js.objects[$0].property(UTF8ToString($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] property when it's a double
    double GetPropertyDouble(std::string name) const {
      return EM_ASM_DOUBLE({
	    return js.objects[$0].property(UTF8ToString($1));
      }, this->id, name.c_str());
    }

    /// @}

    #endif

    /// Append DOM element(s) of the type specified by [name] to this selection.
    Selection Append(std::string name){
      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var new_selection = js.objects[$0].append(UTF8ToString($1));
	    js.objects[$2] = new_selection;
      }, this->id, name.c_str(), new_id);
      return Selection(new_id);
    }

    /// Insert DOM element of type "name" into the current selection before the element selected by
    /// the element specified by the [before] string
    ///
    /// For more information, see the D3 documention on
    /// [insert](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md#insert)
    Selection Insert(std::string name, std::string before=NULL){
      int new_id = NextD3ID();

      if (before.c_str()){
	    EM_ASM_ARGS({
	      var new_sel = js.objects[$0].insert(UTF8ToString($1),
						  UTF8ToString($2));
	      js.objects[$3] = new_sel;
        }, this->id, name.c_str(), before.c_str(), new_id);
      } else {
  	    EM_ASM_ARGS({
	      var new_sel = js.objects[$0].insert(UTF8ToString($1));
	      js.objects[$2] = new_sel;
        }, this->id, name.c_str(), new_id);
      }
      return Selection(new_id);
    }

    /// Create a transition from the current selection. If a [name] is specified
    /// the transition will be given that name
    Transition MakeTransition(std::string name=""){
      int new_id = NextD3ID();
      EM_ASM_ARGS({
 	    var transition = js.objects[$0].transition(UTF8ToString($1));
	    js.objects[$2] = transition;
      }, this->id, name.c_str(), new_id);

      return Transition(new_id);
    }

    Transition MakeTransition(Transition & t){
      int new_id = NextD3ID();
      EM_ASM_ARGS({
 	    var transition = js.objects[$0].transition(js.objects[$1]);
	    js.objects[$2] = transition;
    }, this->id, t.GetID(), new_id);

      return Transition(new_id);
    }


    /// Interrupt the transition with the name [name] on the current selection
    Selection& Interrupt(std::string name=""){
      EM_ASM_ARGS({
	    js.objects[$0].interrupt(UTF8ToString($1));
 	  }, this->id, name.c_str());
      return *this;
    }

    /// Move the elements in this selection by [x] in the x direction and [y]
    /// in the y direction.
    /// Note for advanced users: this method is just a shortcut for setting the
    /// "transform" attribute to "translate(x, y)", because doing that is a pain
    /// in C++ (even more so than in Javascript)
    Selection& Move(int x, int y) {
      EM_ASM_ARGS({
        js.objects[$0].attr("transform", "translate("+$1+","+$2+")");
      }, this->id, x, y);
      return *this;
    }

    /// Rotate the elements in this selection by [degrees].
    /// Note for advanced users: this method is just a shortcut for setting the
    /// "transform" attribute to "rotate(degrees)", because doing that is a pain
    /// in C++ (even more so than in Javascript)
    Selection& Rotate(int degrees) {
      EM_ASM_ARGS({
        js.objects[$0].attr("transform", "rotate("+$1+")");
      }, this->id, degrees);
      return *this;
    }

    /// Change the order of elements in the document to match their order in this selection
    Selection& Order(){
      EM_ASM_ARGS({js.objects[$0].order()},
			this->id);
      return *this;
    }

    Selection& Raise() {
      EM_ASM_ARGS({js.objects[$0].raise();}, this->id);
      return *this;
    }

    Selection& Lower() {
      EM_ASM_ARGS({js.objects[$0].lower();}, this->id);
      return *this;
    }


    /// Listen for an event of type [type] and call [listener] when it happens
    /// [listener] can be a string containing the name of a Javascript function, or a C++ function
    ///
    /// The third paramter for the listener function is the id of a selection containing the
    /// relevant DOM object.
    ///
    /// To remove an event listener, call On with that type and "null" as the listener (default)
    ///
    /// Advanced note: the optional capture flag invokes Javascript's
    /// [useCapture](https://www.w3.org/TR/DOM-Level-2-Events/events.html#Events-registration)
    /// option
    Selection& On(std::string type, std::string listener="null", bool capture=false){

      // Check that the listener is valid
      emp_assert(EM_ASM_INT({
        var func_string = UTF8ToString($0);
        if (func_string == "null") {
          return true;
        }
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }
        return (typeof func_string === "function");
      }, listener.c_str()) \
      && "String passed to On is not s Javascript function or null", listener);

      int new_id = NextD3ID();

      EM_ASM_ARGS({
	    var func_string = UTF8ToString($2);
        if (typeof window[func_string] === "function") {
          func_string = window[func_string];
        }
        for (name in {d3:"d3", emp:"emp"}) {
          if (typeof window[name][func_string] === "function") {
            func_string = window[name][func_string];
          }
        }

	    if (typeof func_string === "function") {
	      js.objects[$0].on(UTF8ToString($1),
		  function(d, i){
		     js.objects[$4] = d3.select(this);
		     func_string(d, i, $4);},
          $3);
	    } else {
	      js.objects[$0].on(UTF8ToString($1), null);
	    }

      }, this->id, type.c_str(), listener.c_str(), capture, new_id);

      return (*this);
    }

    /// @cond TEMPLATES

    /// Version for C++ functions
    template <typename T>
    emp::sfinae_decoy<Selection&, decltype(&T::operator())>
    On(std::string type, T listener, bool capture=false){

      uint32_t fun_id = emp::JSWrap(listener, "", false);
      int new_id = NextD3ID();

      EM_ASM_ARGS({
	      js.objects[$0].on(UTF8ToString($1),
		  function(){
		     js.objects[$4] = d3.select(this);
		     emp.Callback($2, d, i, $4);}, $3);
      }, this->id, type.c_str(), fun_id, capture, new_id);

      emp::JSDelete(fun_id);
      return (*this);
    }

    /// @endcond

    /** Sort the selection by the given comparator function. The function
    can be a C++ function or a stirng indicating a function in the d3 namespace,
    the emp namespace (as results from JSWrapping C++ functions), or the
    window namespace. These three options are checked sequentially in that
    order, so a C++ function with the same name as d3 built-in will not
    override the built-in. Similarly, a function declared directly in the
    window will be overriden by a JSWrapped function with the same name.
    */

    Selection& Sort(std::string comparator = "ascending"){
      D3_CALLBACK_METHOD_1_ARG(sort, comparator.c_str())
      return (*this);
    }

    /// @cond TEMPLATES

    template <typename T>
    emp::sfinae_decoy<Selection&, decltype(&T::operator())>
    Sort(T comparator){
      D3_CALLBACK_METHOD_CPP_FUNCTION_1_ARG(sort, comparator)
      return (*this);
    }

    /// @endcond


    /// @cond DEVELOPERS

    //Set the tool tip up for this selection.
    //This function exists in case you want to bind the tooltip to an
    //event other than mouseover/out
    void SetupToolTip(ToolTip & tip) {
      EM_ASM_ARGS({
       js.objects[$0].call(js.objects[$1]);
   }, this->id, tip.GetID());
    }

    //Tell tooltip to appear on mouseover and dissapear on mouseout
    void BindToolTipMouseover(ToolTip & tip) {
      EM_ASM_ARGS({
       js.objects[$0].on("mouseover", js.objects[$1].show)
                     .on("mouseout", js.objects[$1].hide);
      }, this->id, tip.GetID());
    }

    /// @endcond

    /// Add the ToolTip [tip] to the current selection
    void AddToolTip(ToolTip & tip) {
      SetupToolTip(tip);
      BindToolTipMouseover(tip);
    }

    //TODO:
    //
    //GetClassed()

    //Datum() //requires callbacks

    //Node() //Is the node a selection? Do we even need this?
  };



  /// Create a selection containing the first DOM element matching [selector]
  /// (convenience function to match D3 syntax - you can also just use the constructor)
  Selection Select(std::string selector) {
    return Selection(selector);
  }

  /// Create a selection containing all DOM elements matching [selector]
  /// (convenience function to match D3 syntax - you can also just use the constructor)
  Selection SelectAll(std::string selector) {
    return Selection(selector, true);
  }

  /// Makes a shape of type [shape] for each element in [values] on the first svg canvas on the DOM
  /// Values can be a D3::Dataset, an array, or a vector.
  template<typename T>
  Selection ShapesFromData(T values, std::string shape){
    Selection s = Select("svg").SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

  /// Makes a shape of type [shape] for each element in [values] on [svg], which must be a selection
  /// containing an SVG canvas.
  /// Values can be a D3::Dataset, an array, or a vector.
  template<typename T>
  Selection ShapesFromData(T values, std::string shape, Selection & svg){
    Selection s = svg.SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

}
#endif
