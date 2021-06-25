//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2020.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef __EMP_D3_TRANSITION_H__
#define __EMP_D3_TRANSITION_H__

#include "d3_init.hpp"

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include <array>

#include "../../base/assert.hpp"
#include "../js_utils.hpp"
#include "../JSWrap.hpp"
#include "selection.hpp"

namespace D3 {

  /// [Transitions](https://github.com/d3/d3-3.x-api-reference/blob/master/Transitions.md/)
  /// animated. For additional discussion of transitions in d3, see
  /// are similar to selections, but when you make a change to them (attr or style), it will be
  /// [this article](https://bost.ocks.org/mike/transition/).
  class Transition : public internal::SelectionOrTransition<Transition> {
  public:
    using base_t = internal::SelectionOrTransition<Transition>;

    /**************************************************************************//**
    * @name Constructors
    *
    * Usually transitions are constructed from selections by calling the selection.MakeTransition()
    * method. In rare cases you may want to construct a new transition, though.
    *
    * @{
    ***********************************************/

    /// Default constructor - construct empty transition
    Transition() {;};

    /// Advanced: Construct new transition pointing to the [id]th element in emp_d3.objects.
    Transition(int id) : base_t(id) {;};

    /// Create a transition from the current transition. If a [name] is specified
    /// the transition will be given that name
    ///
    /// Note: In D3.js this method is just called transition(), but in C++ that would cause a
    /// collision with the constructor
    //TODO: D3 supports passing a selection, but it's kind of a weird edge case
    Transition NewTransition(const std::string & name="") const {
      const int new_id = internal::NextD3ID();
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const name = UTF8ToString($1);
        const new_id = $2;
        emp_d3.objects[new_id] = emp_d3.objects[id].transition(name);
      }, this->id, name.c_str(), new_id);

      return Transition(new_id);
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
    Transition & On(const std::string & type, const std::string & listener="null") {

      // Check that the listener is valid
      emp_assert(MAIN_THREAD_EM_ASM_INT({
        var func_string = UTF8ToString($0);
        return emp_d3.is_function(func_string) || func_string=="null";
      }, listener.c_str()) \
      && "String passed to On is neither a Javascript function nor is it null", listener);

      const int new_id = internal::NextD3ID();

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const type = UTF8ToString($1);
        var func_string = UTF8ToString($2);
        const new_id = $3;

        func_string = emp_d3.find_function(func_string);
        if (typeof func_string === "function") {
          emp_d3.objects[id].on(type, func_string);
        } else {
          emp_d3.objects[id].on(type, null);
        }

      }, this->id, type.c_str(), listener.c_str(), new_id);

      return (*this);
    }

    /// @cond TEMPLATES

    /// Version for C++ functions
    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    On(const std::string & type, T listener) {

      const uint32_t fun_id = emp::JSWrap(listener, "", false);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const type = UTF8ToString($1);
        const func_id = $2;
	      emp_d3.objects[id].on(type,
		      function(d, i) { emp.Callback(func_id, d, i); });
      }, this->id, type.c_str(), fun_id);

      emp::JSDelete(fun_id);
      return (*this);
    }

    /// @endcond

    Transition & SetDuration(double time) {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const time = $1;
        emp_d3.objects[id].duration(time);
      }, this->id, time);
      return *this;
    }

    /// Sets special properties of DOM elements on transition (e.g. "checked" for checkboxes)
    /// Value can be a number, function, string, or string naming a Javascript function
    /// See the [d3 documentation](https://github.com/d3/d3-selection#selection_property)
    /// for more information.
    // std::string version
    Transition & SetProperty(const std::string & name, const std::string & value, const std::string & type="end") {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const arg1 = UTF8ToString($1);
        var func_string = UTF8ToString($2);
        const type = UTF8ToString($3);

        func_string = emp_d3.find_function(func_string);
        emp_d3.objects[id].each(type, function() {
          d3.select(this).property(arg1, func_string);
        });
      }, this->id, name.c_str(), value.c_str(), type.c_str());
      return *this;
    }

    /// @cond TEMPLATES

    // Const char * version so raw strings work
    Transition& SetProperty(const std::string & name, const char* value, const std::string & type="end") {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const arg1 = UTF8ToString($1);
        var func_string = UTF8ToString($2);
        const type = UTF8ToString($3);

        func_string = emp_d3.find_function(func_string);
        emp_d3.objects[id].each(type, function() {
          d3.select(this).property(arg1, func_string);
        });
      }, this->id, name.c_str(), value, type.c_str());
      return *this;
    }

    // Generic template version
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, Transition&>::type
    SetProperty(const std::string & name, T value, const std::string & type="end"){
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const arg1 = UTF8ToString($1);
        const value = $2;
        const type = UTF8ToString($3);
        emp_d3.objects[id].each(type, function() {
          d3.select(this).property(arg1, value);
        });
      }, this->id, name.c_str(), value, type.c_str());
      return *this;
    }

    // This version handles values that are C++ functions and wraps them with JSWrap
    // If a function is being used repeatedly, it may be more efficient to wrap it
    // once and then pass the name as a string.
    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    SetProperty(const std::string & name, T value, const std::string & type="end") {
      const uint32_t fun_id = emp::JSWrap(value, "", false);
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const name = UTF8ToString($1);
        const type = UTF8ToString($2);
        const func_id = $3;
        emp_d3.objects[id].each(type, function() {
                d3.select(this).property(name,
                                         function(d, i, j) {
                                          return emp.Callback(func_id, d, i, j);
                                        });
        });
      }, this->id, name.c_str(), value, type.c_str(), fun_id);
      emp::JSDelete(fun_id);
      return *this;
    }

    /// @endcond


    /// Sets this selection's inner html to the specified string, or the string returned by running the
    /// specified function on the element's bound data
    Transition & SetHtml(const std::string & value, const std::string & type="end") {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        var func_string = UTF8ToString($1);
        const type = UTF8ToString($2);
        func_string = emp_d3.find_function(func_string);
        emp_d3.objects[id].each(type, function(){
          d3.select(this).html(func_string);
        });
      }, this->id, value.c_str(), type.c_str());
      return *this;
    }

    /// @cond TEMPLATES

    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    SetHtml(T func, const std::string & type="end") {
      const uint32_t fun_id = emp::JSWrap(func, "", false);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const func_id = $1;
        const type = UTF8ToString($2);
        emp_d3.objects[id].each(type, function(){
                  d3.select(this).html(function(d, i, j) {
                        return emp.Callback(func_id, d, i, j);
                  });
        });
      }, this->id, fun_id, type.c_str());
      emp::JSDelete(fun_id);
      return *this;
    }

    /// @endcond

    /// Change whether or not element in this selection have the [classname] class.
    /// Example: Add the data-point class with selection.SetClassed("data-point", true);
    // Value can also be a function that takes bound data and returns a bool
    Transition & SetClassed(const std::string & classname, bool value, const std::string & type="end") {
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const class = UTF8ToString($1);
        const value = $2;
        const type = UTF8ToString($3);
        emp_d3.objects[id].each(type, function(){
            d3.select(this).classed(class, value);
        });
      }, this->id, classname.c_str(), value, type.c_str());
      return *this;
    }

    ///@cond TEMPLATES

    // Version for C++ function
    template <typename T>
    emp::sfinae_decoy<Transition&, decltype(&T::operator())>
    SetClassed(std::string classname, T func, const std::string & type="end"){
      const uint32_t fun_id = emp::JSWrap(func, "", false);
      MAIN_THREAD_EM_ASM({
        const id = $0;
        const class = UTF8ToString($1);
        const func_id = $2;
        const type = UTF8ToString($3);

        emp_d3.objects[id].each(type, function() {
                  d3.select(this).classed(class,
                                            function(d, i, j) {
                                              return emp.Callback(func_id, d, i, j);
                                          });
        });
      }, this->id, classname.c_str(), fun_id, type.c_str());
      emp::JSDelete(fun_id);

      return *this;
    }

    // Version that allows strings containing function names but warns on other strings
    Transition& SetClassed(const std::string & classname, const std::string & value, const std::string & type="end") {

      emp_assert(MAIN_THREAD_EM_ASM_INT({
        var func_string = UTF8ToString($0);
        return emp_d3.is_function(func_string);
      }, value.c_str()) && "String passed to SetClassed is not a Javascript function", value);

      MAIN_THREAD_EM_ASM({
        const id = $0;
        const arg1 = UTF8ToString($1);
        var func_string = UTF8ToString($2);
        const type = $3;
        func_string = emp_d3.find_function(func_string);
        emp_d3.objects[id].each(type, function() {
          d3.select(this).classed(arg1, func_string);
        });
      }, this->id, classname.c_str(), value.c_str(), type.c_str());

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

    /// Get the value of this object's [name] property when its a string
    std::string GetPropertyString(const std::string & name) const {
      MAIN_THREAD_EM_ASM({
        var text = d3.select(emp_d3.objects[$0]).property(UTF8ToString($1));
        emp.PassStringToCpp(text);
      }, this->id, name.c_str());
      return emp::pass_str_to_cpp();
    }

    /// Get the value of this object's [name] property when it's an int
    int GetPropertyInt(const std::string & name) const {
      return MAIN_THREAD_EM_ASM_INT({
        return d3.select(emp_d3.objects[$0]).property(UTF8ToString($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] property when it's a double
    double GetPropertyDouble(const std::string & name) const {
      return MAIN_THREAD_EM_ASM_DOUBLE({
        return d3.select(emp_d3.objects[$0]).property(UTF8ToString($1));
      }, this->id, name.c_str());
    }

    /// Get this object's html
    std::string GetHtml() const {
      MAIN_THREAD_EM_ASM({
        const text = d3.select(emp_d3.objects[$0]).html();
        emp.PassStringToCpp(text);
      }, this->id);
      return emp::pass_str_to_cpp();
    }

    /// @}

    #endif

  };

}

#endif
