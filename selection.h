#ifndef __SELECTION_H__
#define __SELECTION_H__

#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include "../Empirical/tools/assert.h"
#include <array>
#include "../Empirical/emtools/js_object_struct.h"
#include "../Empirical/emtools/js_utils.h"
#include "utils.h"
#include "dataset.h"
#include "d3_init.h"

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
    Selection(){;}; //Null contstructor
    Selection(int id); //append constructor
    Selection(const char* selector, bool all = false);
    Selection(std::string selector, bool all = false);
    //Selection(Selection* selector, bool all = false); //Do we need this?
    ~Selection(){}; //tilde is creating demangling issues

    Selection Select(const char* selector){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].select(Pointer_stringify($1));
	  js.objects.push(new_selection);
	}, this->id, selector);
      return Selection(new_id);
    }

    Selection Select(std::string selector){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].select(Pointer_stringify($1));
	  js.objects.push(new_selection);
  }, this->id, selector.c_str());
      return Selection(new_id);
    }

    Selection SelectAll(const char* selector){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].selectAll(Pointer_stringify($1));
	  js.objects.push(new_selection);
	}, this->id, selector);
      return Selection(new_id);
    }

    Selection SelectAll(std::string selector){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].selectAll(Pointer_stringify($1));
	  js.objects.push(new_selection);
  }, this->id, selector.c_str());
      return Selection(new_id);
    }

    template <typename T>
    Selection SetAttr(std::string name, T value){
      /* Assigns [value] to the selection's [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      EM_ASM_ARGS({js.objects[$0].attr(Pointer_stringify($1), $2)},
		  this->id, name.c_str(), value);
      return *this;
    }

    Selection SetAttr(std::string name, std::string value){
      /* Assigns [value] to the selections's [name] attribute.

	 This will break if someone happens to use a string that
	 is identical to a function name... but that's unlikely, right?
      */

      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(attr, name.c_str(), value.c_str())
      return *this;
    }

    Selection SetAttr(std::string name, const char * value){
      /* Assigns [value] to the selections's [name] attribute.

	 This will break if someone happens to use a string that
	 is identical to a function name... but that's unlikely, right?
      */

      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(attr, name.c_str(), value)
      return *this;
    }


    template <typename T, size_t SIZE>
    Selection SetAttr(std::string name, std::array<T, SIZE> value){
      /* Assigns [value] to the selections's [name] attribute.

	 This will break if someone happens to use a string that
	 is identical to a function name... but that's unlikely, right?
      */
      emp::pass_array_to_javascript(value);

      EM_ASM_ARGS({
	  js.objects[$0].attr(Pointer_stringify($1), emp_i.__incoming_array);
  }, this->id, name.c_str());

      return *this;
    }

    Selection Append(std::string name){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].append(Pointer_stringify($1));
	  js.objects.push(new_selection);
  }, this->id, name.c_str());
      return Selection(new_id);
    }

    Selection SetStyle(std::string name, const char* value, bool priority=false){
      if (priority){
  	    EM_ASM_ARGS({
	      var in_string = Pointer_stringify($2);
	      var fn = window["emp"][in_string];
	      if (typeof fn === "function"){
	        js.objects[$0].style(Pointer_stringify($1), fn, "important");
	      } else {
	        js.objects[$0].style(Pointer_stringify($1),
				      in_string, "important");
	    }}, this->id, name.c_str(), value);
      } else {
	    CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(style, name.c_str(), value)
      }
      return *this;
    }

    Selection SetStyle(std::string name, std::string value, bool priority=false){
      if (priority){
  	    EM_ASM_ARGS({
	      var in_string = Pointer_stringify($2);
	      var fn = window["emp"][in_string];
	      if (typeof fn === "function"){
	        js.objects[$0].style(Pointer_stringify($1), fn, "important");
	      } else {
	        js.objects[$0].style(Pointer_stringify($1),
				      in_string, "important");
	    }}, this->id, name.c_str(), value.c_str());
      } else {
	    CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(style, name.c_str(), value.c_str())
      }
      return *this;
    }

    template <typename T>
    Selection SetStyle(std::string name, T value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({js.objects[$0].style(Pointer_stringify($1),
	$2, "important")}, this->id, name.c_str(), value);
      }
      else {
	EM_ASM_ARGS({js.objects[$0].style(Pointer_stringify($1),
	$2)}, this->id, name.c_str(), value);
      }
      return *this;
    }

    Selection SetProperty(std::string name, const char* value){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(property, name.c_str(), value)
      return *this;
    }

    Selection SetProperty(std::string name, std::string value){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(property, name.c_str(), value.c_str())
      return *this;
    }

    template <typename T>
    Selection SetProperty(std::string name, T value){
      EM_ASM_ARGS({js.objects[$0].property(Pointer_stringify($1),
					   $2)}, this->id, name.c_str());
      return *this;
    }

    //Option to pass loaded dataset without translating to C++
    Selection Data(Dataset values, std::string key=""){
      int update_id = EM_ASM_INT_V({return js.objects.length});

      EM_ASM_ARGS({
	    var in_string = Pointer_stringify($1);
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

    /*This function appends the specified type of nodes to this
    selection's enter selection, which merges the enter selection
    with the update selection.

    Triggers an assertion error if this selection has no valid enter
    selection.
    */
    Selection EnterAppend(const char* type){

      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->enter);

      EM_ASM_ARGS({
	  var append_selection = js.objects[$0]
	    .enter().append(Pointer_stringify($1));
	  js.objects.push(append_selection);
	    }, this->id, type);

      this->enter = false;

      return Selection(new_id);
    }

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

    Selection ExitRemove(){
      /*Pretty much the only thing you ever want to do with the exit() selection
	is remove all of the nodes in it. This function does just that.

	Triggers an assertion error if this selection has no valid exit
	selection.
       */

      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->exit);

      this->exit = false;

      EM_ASM_ARGS({
	    var exit_selection = js.objects[$0].exit().remove();
	    js.objects.push(exit_selection);
	  }, this->id);
      return Selection(new_id);
    }


    Selection Exit(){
      /* Usually the only thing you want to do with the exit selection
	 is remove its contents, in which case you should use the
	 ExitRemove method. However, advanced users may want to operate
	 on the exit selection, which is why this method is provided.

	 Returns a selection object pointing at this selection's exit selection.
      */

      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->exit);

      EM_ASM_ARGS({
	    var exit_selection = js.objects[$0].exit();
	    js.objects.push(exit_selection);
      }, this->id);
      return Selection(new_id);
    }


    int GetAttrInt(std::string name){
      return EM_ASM_INT({
	    return js.objects[$0].attr(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    double GetAttrDouble(std::string name){
      return EM_ASM_DOUBLE({
	    return js.objects[$0].attr(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    int GetStyleInt(std::string name){
      return EM_ASM_INT({
	    return js.objects[$0].style(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    double GetStyleDouble(std::string name){
      return EM_ASM_INT({
	    return js.objects[$0].style(Pointer_stringify($1));
      }, this->id, name.c_str());
    }

    void SetText(std::string text){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(text, text.c_str())
    }

    void SetHtml(std::string value){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(html, value.c_str())
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

    void Move(int x, int y) {
      EM_ASM_ARGS({
        js.objects[$0].attr("transform", "translate("+$1+","+$2+")");
      }, this->id, x, y);
    }

    void Rotate(int degrees) {
      EM_ASM_ARGS({
        js.objects[$0].attr("transform", "rotate("+$1+")");
      }, this->id, degrees);
    }

    void Remove(){
      EM_ASM_ARGS({js.objects[$0].remove()},
		  this->id);
    }

    bool Empty(){
      int empty = EM_ASM_INT({return Number(js.objects[$0].empty())},
			 this->id);
      return (bool)empty;
    }

    int Size(){
      return EM_ASM_INT({return js.objects[$0].size()},
			 this->id);
    }

    void Order(){
      EM_ASM_ARGS({js.objects[$0].order()},
			this->id);
    }

    void On(std::string type, std::string listener="null", bool capture=false){

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
    }

    char* GetText(){

      int buffer = EM_ASM_INT({
	    var text = js.objects[$0].text();
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
	  }, this->id);

      return (char *)buffer;
    }


    char* GetAttrString(std::string name){
      int buffer = EM_ASM_INT({
	    var text = js.objects[$0].attr(Pointer_stringify($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
      }, this->id, name.c_str());

      return (char *)buffer;
    }


    char* GetStyleString(std::string name){
      int buffer = EM_ASM_INT({
	    var text = js.objects[$0].style(Pointer_stringify($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
      }, this->id, name.c_str());

      return (char *)buffer;
    }


    char* GetHtml(){
      int buffer = EM_ASM_INT({
	    var text = js.objects[$0].html();
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
	  }, this->id);

      return (char *)buffer;
    }

    char* GetProperty(std::string name){
      int buffer = EM_ASM_INT({
	    var text = js.objects[$0].property(Pointer_stringify($1));
	    var buffer = Module._malloc(text.length+1);
	    Module.writeStringToMemory(text, buffer);
	    return buffer;
	  }, this->id, name.c_str());

      return (char *)buffer;
    }

    /* Sort the selection by the given comparator function. The function
    is indicated as a string and can either be in the d3 namespace,
    the emp namespace (as results from JSWrapping C++ functions), or the
    window namespace. These three options are checked sequentially in that
    order, so a C++ function with the same name as d3 built-in will not
    override the built-in. Similarly, a function declared directly in the
    window will be overriden by a JSWrapped function with the same name.
    */

    void Sort(const char * comparator = "ascending"){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(sort, comparator)
    }

    void Sort(std::string comparator = "ascending"){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(sort, comparator.c_str())
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
       CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(each, function.c_str())
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
       CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(each, time.c_str(), function.c_str())
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
    void Call(const char * function){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(call, function)
    }

    void Call(std::string function){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(call, function.c_str())
    }

    /* Returns a new selection, representing the current selection
    filtered by the given string. The string can represent a function
    in either the d3, emp, or window namespaces (as described in Sort),
    or it can be a filter selector. Using a filter selector that somehow
    has the same name as a function in one of the allowed namespaces
    will create a problem, but shouldn't actually be possible.
    */
    Selection Filter(const char * selector){

      int new_id = EM_ASM_INT_V({return js.objects.length});
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(filter, selector)
      StoreNewObject();
      return Selection(new_id);
    }

    Selection Filter(std::string selector){

      int new_id = EM_ASM_INT_V({return js.objects.length});
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(filter, selector.c_str())
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

    //TODO:
    //
    //GetClassed()
    //SetClassed()

    //Datum() //requires callbacks

    //Node() //Is the node a selection? Do we even need this?

    //EnterCall //requires callbacks
    //EnterSelect (when would you actually do that?)



  };

  Selection::Selection(const char* selector, bool all){
    this->enter = false;
    this->exit = false;
    if (all){
      EM_ASM_ARGS({
	  js.objects[$0] =
	  d3.selectAll(Pointer_stringify($1))}, this->id, selector);
    }
    else {
      EM_ASM_ARGS({
	  js.objects[$0] =
	  d3.select(Pointer_stringify($1))}, this->id, selector);
    }
  }

  Selection::Selection(std::string selector, bool all){
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
  }

  Selection::Selection(int id) : D3_Base(id){
    this->enter = false;
    this->exit = false;
  }

Selection Select(const char* selector){
  /*Slightly prettier UI for constructor*/
  return Selection(selector);
}

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
