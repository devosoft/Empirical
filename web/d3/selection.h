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

#include "../../tools/assert.h"
#include "../../web/js_object_struct.h"
#include "../../web/js_utils.h"
#include "../../web/JSWrap.h"

extern "C" {
  extern int n_objects();
}

namespace D3 {

  class Dataset;

  class Selection : public D3_Base {
  private:
    bool enter;
    bool exit;

  public:
    /// Default constructor - constructs empty selection
    Selection(){;};

    /// Create Selection object with a specific id
    /// Advanced note: This is useful when creating a Selection object to point to a selection
    // that you already created in Javascript and added to js.objects.
    Selection(int id) : D3_Base(id){
      this->enter = false;
      this->exit = false;
    };

    /// This is the Selection constructor you usually want to use. It takes a string saying what
    /// to select and a bool saying whether to select all elements matching that string [true] or
    /// just the first [false]
    Selection(std::string selector, bool all = false) {
      this->enter = false;
      this->exit = false;
      if (all){
        EM_ASM_ARGS({
  	  js.objects[$0] =
  	  d3.selectAll(Pointer_stringify($1))}, this->id, selector.c_str());
      }
      else {
        EM_ASM_ARGS({
  	  js.objects[$0] =
  	  d3.select(Pointer_stringify($1))}, this->id, selector.c_str());
      }
    };

    ~Selection(){};

    /// Create a new selection containing the first element matching the [selector] string that are
    /// within this current selection
    Selection Select(std::string selector) {
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	    var new_selection = js.objects[$0].select(Pointer_stringify($1));
	    js.objects.push(new_selection);
      }, this->id, selector.c_str());
      return Selection(new_id);
    }

    /// Create a new selection containing all elements matching the [selector] string that are
    /// within this current selection
    Selection SelectAll(std::string selector) {
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	    var new_selection = js.objects[$0].selectAll(Pointer_stringify($1));
	    js.objects.push(new_selection);
      }, this->id, selector.c_str());
      return Selection(new_id);
    }

    /// Append DOM element(s) of the type specified by [name] to this selection.
    Selection Append(std::string name){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      EM_ASM_ARGS({
	    var new_selection = js.objects[$0].append(Pointer_stringify($1));
	    js.objects.push(new_selection);
      }, this->id, name.c_str());
      return Selection(new_id);
    }


    /// Bind data to selection. Accepts any contiguous container (such as an array or
    /// vector), or a D3::Dataset object (which stores the data Javascript). Optionally
    /// also accepts a key function to run on each element to determine which elements are
    /// equivalent (if no key is provided, elements are expected to be in the same order
    /// each time you bind data to this selection). This function can either be a string
    /// with the name of a function in Javascript, or it can be a C++ function pointer,
    /// std::function object, or lambda.

    //Option to pass loaded dataset stored in Javascript without translating to C++
    Selection Data(Dataset values, std::string key=""){
      int update_id = EM_ASM_INT_V({return js.objects.length});

      EM_ASM_ARGS({
        //We could make this slightly prettier with macros, but would
        //add an extra comparison
	    var func_string = Pointer_stringify($1);
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

	    js.objects.push(update_sel);
      },this->id, key.c_str(), values.GetID());

      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }

    // Accepts Dataset and C++ function as key
    template<typename T>
    typename emp::sfinae_decoy<Selection, decltype(&T::operator())>::type
    Data(Dataset values, T key){
      int update_id = EM_ASM_INT_V({return js.objects.length});
      uint32_t fun_id = emp::JSWrap(key, "", false);

  	  EM_ASM_ARGS({
        var update_sel = js.objects[$0].data(js.objects[$2],
                                                function(d,i,k) {
                                                  return emp.Callback($1, d, i, k);
                                            });
	    js.objects.push(update_sel);
      },this->id, fun_id, values.GetID());

      emp::JSDelete(fun_id);

      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }

    // Accepts string referring to Javascript function
    template<typename C, class = typename C::value_type>
    Selection Data(C values, std::string key=""){
      int update_id = EM_ASM_INT_V({return js.objects.length});
      emp::pass_array_to_javascript(values);

  	  EM_ASM_ARGS({
	    var in_string = Pointer_stringify($1);
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

	    js.objects.push(update_sel);
      },this->id, key.c_str());

      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }

    // Accepts C++ function as key
    template<typename C, class = typename C::value_type, typename T>
    typename emp::sfinae_decoy<Selection, decltype(&T::operator())>::type
    Data(C values, T key){
      int update_id = EM_ASM_INT_V({return js.objects.length});
      emp::pass_array_to_javascript(values);
      uint32_t fun_id = emp::JSWrap(key, "", false);

  	  EM_ASM_ARGS({
        var update_sel = js.objects[$0].data(emp_i.__incoming_array,
                                                function(d,i,k) {
                                                  return emp.Callback($1, d, i, k);
                                            });
	    js.objects.push(update_sel);
      },this->id, fun_id);

      emp::JSDelete(fun_id);

      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }


    /// This function appends the specified type of nodes to this
    /// selection's enter selection, which merges the enter selection
    /// with the update selection.
    ///
    /// Selection must have an enter selection (i.e. have just had data bound to it).

    Selection EnterAppend(std::string type){

      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->enter);

      EM_ASM_ARGS({
	    var append_selection = js.objects[$0].enter()
                               .append(Pointer_stringify($1));
	    js.objects.push(append_selection);
      }, this->id, type.c_str());

      this->enter = false;

      return Selection(new_id);
    }

    /// Pretty much the only thing you ever want to do with the exit() selection
    /// is remove all of the nodes in it. This function does just that.

    ///Selection must have an exit selection (i.e. have just had data bound to it).
    Selection ExitRemove(){

      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->exit);

      this->exit = false;

      EM_ASM_ARGS({
	    var exit_selection = js.objects[$0].exit().remove();
	    js.objects.push(exit_selection);
	  }, this->id);
      return Selection(new_id);
    }

    /// Usually the only thing you want to do with the exit selection
    /// is remove its contents, in which case you should use the
    /// ExitRemove method. However, advanced users may want to operate
    /// on the exit selection, which is why this method is provided.
    ///
    /// Returns a selection object pointing at this selection's exit selection.
    Selection Exit(){

      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->exit);

      EM_ASM_ARGS({
	    var exit_selection = js.objects[$0].exit();
	    js.objects.push(exit_selection);
      }, this->id);
      return Selection(new_id);
    }

    Selection Transition(std::string name=""){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
 	    var transition = js.objects[$0].transition(Pointer_stringify($1));
	    js.objects.push(transition);
  	  }, this->id, name.c_str());
      return Selection(new_id);
    }

    void Interrupt(std::string name=""){
      EM_ASM_ARGS({
	    js.objects[$0].interrupt(Pointer_stringify($1));
 	  }, this->id, name.c_str());
    }

    Selection Insert(std::string name, std::string before=NULL){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      if (before.c_str()){
	    EM_ASM_ARGS({
	      var new = js.objects[$0].insert(Pointer_stringify($1),
						  Pointer_stringify($2));
	      js.objects.push(new);
        }, this->id, name.c_str(), before.c_str());
      } else {
  	    EM_ASM_ARGS({
	      var new = js.objects[$0].insert(Pointer_stringify($1));
	      js.objects.push(new);
	    }, this->id, name.c_str());
      }
      return Selection(new_id);
    }


    Selection EnterInsert(std::string name, std::string before=NULL){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->enter);

      if (before.c_str()){
	    EM_ASM_ARGS({
	      var new = js.objects[$0].enter().insert(Pointer_stringify($1),
						  Pointer_stringify($2));
	      js.objects.push(new);
	    }, this->id, name.c_str(), before.c_str());
      } else {
	    EM_ASM_ARGS({
	      var new = js.objects[$0].enter().insert(Pointer_stringify($1));
	      js.objects.push(new);
	    }, this->id, name.c_str());
      }
      this->enter = false;
      return Selection(new_id);
    }

    /// Move the elements in this selection by [x] in the x direction and [y]
    /// in the y direction.
    /// Note for advanced users: this method is just a shortcut for setting the
    /// "transform" attribute to "translate(x, y)", because doing that is a pain
    /// in C++ (even more so than in Javascript)
    void Move(int x, int y) {
      EM_ASM_ARGS({
        js.objects[$0].attr("transform", "translate("+$1+","+$2+")");
      }, this->id, x, y);
    }

    /// Rotate the elements in this selection by [degrees].
    /// Note for advanced users: this method is just a shortcut for setting the
    /// "transform" attribute to "rotate(degrees)", because doing that is a pain
    /// in C++ (even more so than in Javascript)
    void Rotate(int degrees) {
      EM_ASM_ARGS({
        js.objects[$0].attr("transform", "rotate("+$1+")");
      }, this->id, degrees);
    }

    /// Remove the elements in this selection from the document
    void Remove(){
      EM_ASM_ARGS({js.objects[$0].remove()},
		  this->id);
    }

    /// Returns true if there are no elements in this selection (or all elements are null)
    bool Empty(){
      int empty = EM_ASM_INT({return Number(js.objects[$0].empty())},
			 this->id);
      return (bool)empty;
    }

    /// Returns number of elements in this selection
    int Size(){
      return EM_ASM_INT({return js.objects[$0].size()},
			 this->id);
    }

    /// Change the order of elements in the document to match their order in this selection
    void Order(){
      EM_ASM_ARGS({js.objects[$0].order()},
			this->id);
    }

    Selection On(std::string type, std::string listener="null", bool capture=false){

      EM_ASM_ARGS({
	    var in_string = Pointer_stringify($2);
	    var fn = window["emp"][in_string];
	    if (typeof fn === "function"){
	      js.objects[$0].on(Pointer_stringify($1),
		  function(){
		     var new_id = js.objects.length;
		     js.objects.push(d3.select(this));
		     fn(new_id);}, $3);
	    } else {
	      //if this isn't the name of a function, then it should be null
	      //otherwise, the user passed an invalid listener
	      //emp_assert(in_string == "null");

	      js.objects[$0].on(Pointer_stringify($1), null);
	    }

      }, this->id, type.c_str(), listener.c_str(), capture);

      return (*this);
    }

    /**************************************************************************//**
    * Setters - Methods for setting values on the current selection
    *
    * There are three main types of values you might want to change about a selection:
    * attributes (use `SetAttr`), styles (use `SetStyle`), and properties (use `SetProperty`)
    * The distinction between these types is rooted in how they are represented in web languages
    * (Javascript, CSS, and HTML) and would ideally be abstracted in this wrapper.
    *
    * Advanced note: In D3.js, the same functions are used to set and get values (depending on
    * whether an argument is passed). Because C++ needs to have clearly defined
    * return types we need separate getters for each return type.
    ***********************************************/

    /** Assigns [value] to the selection's [name] attribute. Value can be any primitive
    type, a string, a function object, or a lambda. If a string is passed, it can be a normal string, or
    the name of a function in d3, emp (such as one created with JSWrap), or the local window.
    If it is a function name, that function will be run, receiving bound data, if any, as input*/

    //This version handles values that are not functions or strings
    //is_fundamental safely excludes lambdas
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, Selection>::type
    SetAttr(std::string name, T value) {

      EM_ASM_ARGS({js.objects[$0].attr(Pointer_stringify($1), $2)},
		  this->id, name.c_str(), value);
      return *this;
    }

    //This version handles values that are C++ functions and wraps them with JSWrap
    //If a function is being used repeatedly, it may be more efficient to wrap it
    //once and then pass the name as a string.
    template <typename T>
    typename emp::sfinae_decoy<Selection, decltype(&T::operator())>::type
    SetAttr(std::string name, T value) {

      //This should probably be JSWrapOnce, but that breaks the visualization
      uint32_t fun_id = emp::JSWrap(value, "", false);

      EM_ASM_ARGS({
        js.objects[$0].attr(Pointer_stringify($1), function(d, i, k) {
                                                      return emp.Callback($2, d, i, k);
                                                    });
      }, this->id, name.c_str(), fun_id);

      emp::JSDelete(fun_id);

      return *this;
    }

    /* Version for strings
    This will break if someone happens to use a string that
    is identical to a function name
    */

    Selection SetAttr(std::string name, std::string value) {

      D3_CALLBACK_METHOD_2_ARGS(attr, name.c_str(), value.c_str())
      return *this;
    }

    /* We also need a const char * version, because the template version will be a
    better match for raw strings than the std::string version
    */

    Selection SetAttr(std::string name, const char * value) {

      D3_CALLBACK_METHOD_2_ARGS(attr, name.c_str(), value)
      return *this;
    }

    /* Version for containers */
    template <typename T>
    typename emp::sfinae_decoy<Selection, typename T::value_type>::type
    SetAttr(std::string name, T value) {
      emp::pass_array_to_javascript(value);

      EM_ASM_ARGS({
	    js.objects[$0].attr(Pointer_stringify($1), emp_i.__incoming_array);
      }, this->id, name.c_str());

      return *this;
    }

    /** Sets the selection's [name] style to [value]. This is the same idea as
    SetAttr, except for CSS styles. Value can be any primitive
    type, a string, a function object, or a lambda. If a string is passed, it can be a normal string, or
    the name of a function in d3, emp (such as one created with JSWrap), or the local window.
    If it is a function name, that function will be run, receiving bound data, if any, as input

    There is a third optional argument, a boolean indicating whether you want to give
    this setting priority.
    */

    //Const char * version so the generic template doesn't get greedy with
    //string literals
    Selection SetStyle(std::string name, const char* value, bool priority=false){
      if (priority){
  	    EM_ASM_ARGS({
	      var func_string = Pointer_stringify($2);
	      CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW("emp", "d3");
	      js.objects[$0].style(Pointer_stringify($1), in_string, "important");
	    }, this->id, name.c_str(), value);
      } else {
	    D3_CALLBACK_METHOD_2_ARGS(style, name.c_str(), value)
      }
      return *this;
    }

    //std::string version because std::strings are better
    Selection SetStyle(std::string name, std::string value, bool priority=false){
      if (priority){
  	    EM_ASM_ARGS({
          var func_string = Pointer_stringify($2);
  	      CONVERT_FUNCSTRING_TO_FUNCTION_IF_IN_NAMESPACE_OR_WINDOW("emp", "d3");
  	      js.objects[$0].style(Pointer_stringify($1), in_string, "important");
	    }, this->id, name.c_str(), value.c_str());
      } else {
	    D3_CALLBACK_METHOD_2_ARGS(style, name.c_str(), value.c_str())
      }
      return *this;
    }

    //This version handles values that are C++ functions and wraps them with JSWrap
    //If a function is being used repeatedly, it may be more efficient to wrap it
    //once and then pass the name as a string.
    template <typename T>
    typename emp::sfinae_decoy<Selection, decltype(&T::operator())>::type
    SetStyle(std::string name, T value) {
      D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(style, name.c_str(), value);
      return *this;
    }

    //Generic template version
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, Selection>::type
    SetStyle(std::string name, T value, bool priority=false){
      if (priority){
	    EM_ASM_ARGS({js.objects[$0].style(Pointer_stringify($1), $2, "important")},
            this->id, name.c_str(), value);
        }
      else {
	    EM_ASM_ARGS({js.objects[$0].style(Pointer_stringify($1), $2)},
            this->id, name.c_str(), value);
      }
      return *this;
    }

    /// Sets special properties of DOM elements (e.g. "checked" for checkboxes)
    /// See the (d3 documentation)[https://github.com/d3/d3-selection#selection_property]
    /// for more information.

    // Const char * version so raw strings work
    Selection SetProperty(std::string name, const char* value){
      D3_CALLBACK_METHOD_2_ARGS(property, name.c_str(), value)
      return *this;
    }

    // std::string verison
    Selection SetProperty(std::string name, std::string value){
      D3_CALLBACK_METHOD_2_ARGS(property, name.c_str(), value.c_str())
      return *this;
    }

    //Generic template version
    template <typename T>
    typename std::enable_if<std::is_fundamental<T>::value, Selection>::type
    SetProperty(std::string name, T value){
      EM_ASM_ARGS({js.objects[$0].property(Pointer_stringify($1),
					   $2)}, this->id, name.c_str());
      return *this;
    }

    //This version handles values that are C++ functions and wraps them with JSWrap
    //If a function is being used repeatedly, it may be more efficient to wrap it
    //once and then pass the name as a string.
    template <typename T>
    typename emp::sfinae_decoy<Selection, decltype(&T::operator())>::type
    SetProperty(std::string name, T value) {
      D3_CALLBACK_METHOD_CPP_FUNCTION_2_ARGS(property, name.c_str(), value);
      return *this;
    }

    /// Sets this selection's text to the specified string
    void SetText(std::string text){
      D3_CALLBACK_METHOD_1_ARG(text, text.c_str())
    }

    /// Sets this selection's inner html to the specified string
    void SetHtml(std::string value){
      D3_CALLBACK_METHOD_1_ARG(html, value.c_str())
    }

    /**************************************************************************//**
    * Getters - Functions for getting values that can be set with the setters
    *
    * Advanced note: In D3.js, the same functions are used to set and get values (depending on
    * whether an argument is passed). Because C++ needs to have clearly defined
    * return types (and because different macros are required to return different types from
    * Javascript), we need separate getters for each return type.
    ***********************************************/

    /// Get the value of this object's [name] attribute when it's a string
    std::string GetAttrString(std::string name){
      char * buffer = (char *)EM_ASM_INT({
	    var text = js.objects[$0].attr(Pointer_stringify($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
      }, this->id, name.c_str());

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] attribute when it's an int
    int GetAttrInt(std::string name){
      return EM_ASM_INT({
	    return js.objects[$0].attr(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] attribute when it's a double
    double GetAttrDouble(std::string name){
      return EM_ASM_DOUBLE({
	    return js.objects[$0].attr(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] style when it's a string
    std::string GetStyleString(std::string name){
      char * buffer = (char *)EM_ASM_INT({
	    var text = js.objects[$0].style(Pointer_stringify($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
      }, this->id, name.c_str());

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] style when it's an int
    int GetStyleInt(std::string name){
      return EM_ASM_INT({
	    return js.objects[$0].style(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] style when it's a double
    double GetStyleDouble(std::string name){
      return EM_ASM_DOUBLE({
	    return js.objects[$0].style(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    /// Get this object's text
    std::string GetText(){

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

    /// Get this object's html
    std::string GetHtml(){
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
    std::string GetPropertyString(std::string name){
      char * buffer = (char *)EM_ASM_INT({
	    var text = js.objects[$0].property(Pointer_stringify($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
	  }, this->id, name.c_str());

      std::string result = std::string(buffer);
      free(buffer);
      return result;
    }

    /// Get the value of this object's [name] property when it's an int
    int GetPropertyInt(std::string name){
      return EM_ASM_INT({
	    return js.objects[$0].property(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    /// Get the value of this object's [name] property when it's a double
    double GetPropertyDouble(std::string name){
      return EM_ASM_DOUBLE({
	    return js.objects[$0].property(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    /* Sort the selection by the given comparator function. The function
    is indicated as a string and can either be in the d3 namespace,
    the emp namespace (as results from JSWrapping C++ functions), or the
    window namespace. These three options are checked sequentially in that
    order, so a C++ function with the same name as d3 built-in will not
    override the built-in. Similarly, a function declared directly in the
    window will be overriden by a JSWrapped function with the same name.
    */

    void Sort(std::string comparator = "ascending"){
      D3_CALLBACK_METHOD_1_ARG(sort, comparator.c_str())
    }

    /* Call the given function on each element of the selection. The function
    is indicated as a string and can either be in the d3 namespace,
    the emp namespace (as results from JSWrapping C++ functions), or the
    window namespace. These three options are checked sequentially in that
    order, so a C++ function with the same name as d3 built-in will not
    override the built-in. Similarly, a function declared directly in the
    window will be overriden by a JSWrapped function with the same name.
     */

     void Each(std::string function){
       D3_CALLBACK_METHOD_1_ARG(each, function.c_str())
     }

     /* Call the given function on each element of the selection. The function
    is indicated as a string and can either be in the d3 namespace,
    the emp namespace (as results from JSWrapping C++ functions), or the
    window namespace. These three options are checked sequentially in that
    order, so a C++ function with the same name as d3 built-in will not
    override the built-in. Similarly, a function declared directly in the
    window will be overriden by a JSWrapped function with the same name.
      */

     void Each(std::string time, std::string function){
       D3_CALLBACK_METHOD_2_ARGS(each, time.c_str(), function.c_str())
     }

     /* Call the given function once on the entire selection. The function
    is indicated as a string and can either be in the d3 namespace,
    the emp namespace (as results from JSWrapping C++ functions), or the
    window namespace. These three options are checked sequentially in that
    order, so a C++ function with the same name as d3 built-in will not
    override the built-in. Similarly, a function declared directly in the
    window will be overriden by a JSWrapped function with the same name.

    TODO: Allow arguments
      */

    void Call(std::string function){
      D3_CALLBACK_METHOD_1_ARG(call, function.c_str())
    }

    /* Returns a new selection, representing the current selection
    filtered by the given string. The string can represent a function
    in either the d3, emp, or window namespaces (as described in Sort),
    or it can be a filter selector. Using a filter selector that somehow
    has the same name as a function in one of the allowed namespaces
    will create a problem, but shouldn't actually be possible.
    */

    Selection Filter(std::string selector){

      int new_id = EM_ASM_INT_V({return js.objects.length});
      D3_CALLBACK_METHOD_1_ARG(filter, selector.c_str())
      StoreNewObject();
      return Selection(new_id);
    }

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

    //Convenience function for most common tooltip setup
    void AddToolTip(ToolTip & tip) {
      SetupToolTip(tip);
      BindToolTipMouseover(tip);
    }

    void SetClassed(std::string classname, bool value) {
      EM_ASM_ARGS({
        js.objects[$0].classed(Pointer_stringify($1), $2);
      }, this->id, classname.c_str(), value);
    }

    //TODO:
    //
    //GetClassed()

    //Datum() //requires callbacks

    //Node() //Is the node a selection? Do we even need this?

    //EnterCall //requires callbacks
    //EnterSelect (when would you actually do that?)



  };




Selection Select(std::string selector){
  /*Slightly prettier UI for constructor*/
  return Selection(selector);
}

Selection SelectAll(const char* selector){
  /*Slightly prettier UI for constructor*/
  return Selection(selector, true);
}

Selection SelectAll(std::string selector){
  /*Slightly prettier UI for constructor*/
  return Selection(selector, true);
}

  template<std::size_t SIZE>
  Selection ShapesFromData(std::array<int32_t, SIZE> values, const char* shape){
    Selection s = Select("svg").SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

  template<std::size_t SIZE>
  Selection ShapesFromData(std::array<int32_t, SIZE> values, std::string shape){
    Selection s = Select("svg").SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

  Selection ShapesFromData(Dataset values, const char* shape){
    Selection s = Select("svg").SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

  Selection ShapesFromData(Dataset values, std::string shape){
    Selection s = Select("svg").SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

}
#endif
