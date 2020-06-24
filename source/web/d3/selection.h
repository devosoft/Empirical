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

  // Forward-declare dataset and transition
  class Dataset;
  class Transition;

  /// [Selections](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md/)
  /// are the primary way that d3 allows you to operate on DOM elements
  /// (i.e. objects on your webpage). A selection is effectively an array of DOM elements
  /// that you can act on at the same time and bind a collection of data to.
  ///
  /// For a deep dive into how selections work in d3, see
  /// [this article](https://bost.ocks.org/mike/selection/).
  class Selection : public internal::SelectionOrTransition<Selection> {

  public:
    using base_t = typename internal::SelectionOrTransition<Selection>;
    /// @name Constructors
    /// You may prefer to use the Select or SelectAll functions for improved
    /// code clarity/consistency with d3.js
    /// @{

    /// Default constructor - constructs empty selection
    Selection() {
        EM_ASM({emp_d3.objects[$0] = d3.selection();}, this->id);
    };

    /// Create Selection object with a specific id.
    ///
    /// Advanced note: This is useful when creating a Selection object to point to a selection
    // that you already created in Javascript and added to js.objects.
    Selection(int id) : base_t(id) {;};

    Selection(const Selection & s) : base_t(s) {;};

    /// This is the Selection constructor you usually want to use. It takes a string saying what
    /// to select and a bool saying whether to select all elements matching that string [true] or
    /// just the first [false]
    Selection(const std::string & selector, bool all = false) {
      if (all) {
        EM_ASM({
          emp_d3.objects[$0] = d3.selectAll(UTF8ToString($1));
        }, this->id, selector.c_str());
      }
      else {
        EM_ASM({
          emp_d3.objects[$0] = d3.select(UTF8ToString($1));
        }, this->id, selector.c_str());
      }
    };

    /// Destructor
    ~Selection() {};

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

    // Option to pass loaded dataset stored in Javascript without translating to C++
    Selection Data(Dataset & values, const std::string & key=""){
      const int new_id = NextD3ID();

      EM_ASM({
        //We could make this slightly prettier with macros, but would
        //add an extra comparison
        const selection_id = $0;
        const in_string = UTF8ToString($1);
        const data_id = $2;
        const new_id = $3;

        var fn = emp_d3.find_function(in_string);
        if (typeof fn === "function") {
          var update_sel = emp_d3.objects[selection_id].data(emp_d3.objects[data_id], fn);
        } else {
          var update_sel = emp_d3.objects[selection_id].data(emp_d3.objects[data_id]);
        }

        emp_d3.objects[new_id] = update_sel;
      },this->id, key.c_str(), values.GetID(), new_id);

      return Selection(new_id);
    }

    ///@cond TEMPLATES

    // Accepts Dataset and C++ function as key
    template<typename T>
    emp::sfinae_decoy<Selection, decltype(&T::operator())>
    Data(Dataset & values, T key){
      const int new_id = NextD3ID();
      const uint32_t fun_id = emp::JSWrap(key, "", false);

      EM_ASM({
        const selection_id = $0;
        const func_id = $1;
        const data_id = $2;
        const new_id = $3;
        emp_d3.objects[new_id] = emp_d3.objects[selection_id].data(emp_d3.objects[data_id],
                                                function(d,i) {
                                                  return emp.Callback(func_id, d, i);
                                                });

      }, this->id, fun_id, values.GetID(), new_id);

      emp::JSDelete(fun_id);
      return Selection(update_id);
    }

    // Accepts string referring to Javascript function
    template<typename C, class = typename C::value_type>
    Selection Data(const C & values, const std::string & key="") {
      const int new_id = NextD3ID();

      emp::pass_array_to_javascript(values); // This passes arbitrary container data into js

      EM_ASM({
        const selection_id = $0;
        const in_string = UTF8ToString($1);
        const new_id = $2;

        var fn = emp_d3.find_function(in_string);

        if (typeof fn === "function"){
          var update_sel = emp_d3.objects[selection_id].data(emp_i.__incoming_array, fn);
        } else {
          var update_sel = emp_d3.objects[selection_id].data(emp_i.__incoming_array);
        }

        emp_i.__incoming_array = [];

        emp_d3.objects[new_id] = update_sel;
      }, this->id, key.c_str(), new_id);

      return Selection(new_id);
    }

    // Accepts C++ function as key
    template<typename C, class = typename C::value_type, typename T>
    emp::sfinae_decoy<Selection, decltype(&T::operator())>
    Data(const C & values, T key){
      const int new_id = NextD3ID();
      emp::pass_array_to_javascript(values);
      const uint32_t fun_id = emp::JSWrap(key, "", false);

      EM_ASM({
        const selection_id = $0;
        const func_id = $1;
        const new_id = $2;
        var update_sel = emp_d3.objects[selection_id].data(emp_i.__incoming_array,
                                                function(d,i,k) {
                                                  return emp.Callback(func_id, d, i, k);
                                                });
        emp_d3.objects[new_id] = update_sel;
        emp_i.__incoming_array = []

      }, this->id, fun_id, new_id);

      emp::JSDelete(fun_id);
      return Selection(new_id);
    }

    /// @endcond

    Dataset GetData() const {
        const int new_id = NextD3ID();
        EM_ASM({
            emp_d3.objects[$1] = [emp_d3.objects[$0].data()];
        }, this->id, new_id);
        return Dataset(new_id);
    }

    // TODO?: GetDataAsBlah

    // TODO - support versions of Append and Insert that take functions as arguments
    // https://github.com/d3/d3-selection#selection_data
    // If the specified type is a function, it is evaluated for each selected element, in order, being passed the current datum (d), the current index (i), and the current group (nodes), with this as the current DOM element (nodes[i]). This function should return an element to be appended

    /// Sometimes you want to perform multiple operations on the enter selection. If so, you can
    /// use the Enter() method to get the enter selection, rather than using one of the convenience
    /// functions like EnterAppend().
    ///
    /// Returns a selection object pointing at this selection's enter selection.
    Selection Enter() {
      const int new_id = NextD3ID();

      EM_ASM({
	      var enter_selection = emp_d3.objects[$0].enter();
	      emp_d3.objects[$1] = enter_selection;
      }, this->id, new_id);

      return Selection(new_id);
    }

    /// Usually the only thing you want to do with the exit selection
    /// is remove its contents, in which case you should use the
    /// ExitRemove method. However, advanced users may want to operate
    /// on the exit selection, which is why this method is provided.
    ///
    /// Returns a selection object pointing at this selection's exit selection.
    Selection Exit() {
      const int new_id = NextD3ID();

      EM_ASM({
        var exit_selection = emp_d3.objects[$0].exit();
        emp_d3.objects[$1] = exit_selection;
      }, this->id, new_id);

      return Selection(new_id);
    }

    /// Append DOM element(s) of the type specified by [name] to this selection.
    Selection Append(const std::string & name) {
      const int new_id = NextD3ID();

      EM_ASM({
        var new_selection = emp_d3.objects[$0].append(UTF8ToString($1));
        emp_d3.objects[$2] = new_selection;
      }, this->id, name.c_str(), new_id);
      return Selection(new_id);
    }

    /// Insert DOM element of type "name" into the current selection before the element selected by
    /// the element specified by the [before] string
    ///
    /// For more information, see the D3 documention on
    /// [insert](https://github.com/d3/d3-3.x-api-reference/blob/master/Selections.md#insert)
    Selection Insert(const std::string & name, const std::string & before=NULL){
      int new_id = NextD3ID();

      if (before.c_str()){
        EM_ASM({
          var new_sel = emp_d3.objects[$0].insert(UTF8ToString($1), UTF8ToString($2));
          emp_d3.objects[$3] = new_sel;
          }, this->id, name.c_str(), before.c_str(), new_id);
      } else {
  	    EM_ASM({
	      var new_sel = emp_d3.objects[$0].insert(UTF8ToString($1));
	      emp_d3.objects[$2] = new_sel;
        }, this->id, name.c_str(), new_id);
      }
      return Selection(new_id);
    }

    /// This function appends the specified type of nodes to this
    /// selection's enter selection, which merges the enter selection
    /// with the update selection.
    ///
    /// Selection must have an enter selection (i.e. have just had data bound to it).
    Selection EnterAppend(const std::string & type) {

      const int new_id = NextD3ID();

      EM_ASM({
        const selection_id = $0;
        const type_str = UTF8ToString($1);
        const new_id = $2;
        var append_selection = emp_d3.objects[selection_id].enter()
                                .append(type_str);
        emp_d3.objects[new_id] = append_selection;
      }, this->id, type.c_str(), new_id);

      return Selection(new_id);
    }

    /// Pretty much the only thing you ever want to do with the exit() selection
    /// is remove all of the nodes in it. This function does just that.

    ///Selection must have an exit selection (i.e. have just had data bound to it).
    void ExitRemove() {
      const int new_id = NextD3ID();

      EM_ASM({
	      var exit_selection = emp_d3.objects[$0].exit().remove();
	      emp_d3.objects[$1] = exit_selection;
      }, this->id, new_id);
    }

    /// Insert elements of type [name] into current enter selection
    ///
    /// For more information, see the D3 documention on
    /// [insert](https://github.com/d3/d3-selection#selection_insert)
    Selection EnterInsert(const std::string & name, const std::string & before=NULL) {
      const int new_id = NextD3ID();

      if (before.c_str()) {
        EM_ASM({
          var new_sel = emp_d3.objects[$0].enter().insert(UTF8ToString($1),
                UTF8ToString($2));
          emp_d3.objects[$3] = new_sel;
        }, this->id, name.c_str(), before.c_str(), new_id);
      } else {
        EM_ASM({
          var new_sel = emp_d3.objects[$0].enter().insert(UTF8ToString($1));
          emp_d3.objects[$2] = new_sel;
        }, this->id, name.c_str(), new_id);
      }
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
    // std::string version
    Selection & SetProperty(const std::string & name, const std::string & value){
      D3_CALLBACK_METHOD_2_ARGS(property, name.c_str(), value.c_str())
      return *this;
    }







  };

}

#endif
