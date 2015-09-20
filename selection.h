#ifndef __SELECTION_H__
#define __SELECTION_H__

#include <emscripten.h>
#include <iostream>
#include <string>
#include <typeinfo>
#include <map>
#include "../Empirical/tools/assert.h"
#include <array>
#include "../Empirical/emtools/js_object_struct.h"
#include "../Empirical/emtools/js_utils.h"
#include "utils.h"

extern "C" {
  extern int n_objects();
}

namespace D3 {

  class Selection{
  private:
    int id;
    bool enter;
    bool exit;

  public:
    Selection(int id); //append constructor
    Selection(const char* selector, bool all = false);
    //Selection(Selection* selector, bool all = false); //Do we need this?
    //~Selection(); //tilde is creating demangling issues

    int GetID() {
      return this->id;
    }

    Selection Select(const char* selector){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].select(Pointer_stringify($1));
	  js.objects.push(new_selection);
	}, this->id, selector);
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

    template <typename T>
    Selection SetAttr(const char* name, T value){
      /* Assigns [value] to the selection's [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      EM_ASM_ARGS({js.objects[$0].attr(Pointer_stringify($1), $2)},
		  this->id, name, value);
      return *this;
    }

    Selection SetAttr(const char* name, const char* value){
      /* Assigns [value] to the selections's [name] attribute.
	 
	 This will break if someone happens to use a string that
	 is identical to a function name... but that's unlikely, right?
      */

      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(attr, name, value)
      return *this;
    }

    template <typename T, size_t SIZE>
    Selection SetAttr(const char* name, std::array<T, SIZE> value){
      /* Assigns [value] to the selections's [name] attribute.
	 
	 This will break if someone happens to use a string that
	 is identical to a function name... but that's unlikely, right?
      */
      emp::pass_array_to_javascript(value);

      EM_ASM_ARGS({
	  js.objects[$0].attr(Pointer_stringify($1), emp.__incoming_array);
	}, this->id, name);

      return *this;
    }

    Selection Append(const char* name){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      EM_ASM_ARGS({
	  var new_selection = js.objects[$0].append(Pointer_stringify($1));
	  js.objects.push(new_selection);
	}, this->id, name);
      return Selection(new_id);
    }
    
    Selection SetStyle(const char* name, const char* value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({
	    var in_string = Pointer_stringify($2);
	    var fn = window["emp"][in_string];
	    if (typeof fn === "function"){
	      js.objects[$0].style(Pointer_stringify($1), fn, "important");
	    } else {
	      js.objects[$0].style(Pointer_stringify($1), 
				      in_string, "important");
	    }}, this->id, name, value);
      }
      else {
	CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(style, name, value)
      }
      return *this;
    }

    template <typename T>
    Selection SetStyle(const char* name, T value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({js.objects[$0].style(Pointer_stringify($1), 
	$2, "important")}, this->id, name, value);
      }
      else {
	EM_ASM_ARGS({js.objects[$0].style(Pointer_stringify($1), 
	$2)}, this->id, name, value);
      }
      return *this;
    }

    Selection SetProperty(const char* name, const char* value){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_2_ARGS(property, name, value)
      return *this;
    }

    template <typename T>
    Selection SetProperty(const char* name, T value){
      EM_ASM_ARGS({js.objects[$0].property(Pointer_stringify($1), 
					   $2)}, this->id, name);
      return *this;
    }

    template<std::size_t SIZE, typename T>
    Selection Data(std::array<T, SIZE> values, const char* key=""){
      int update_id = EM_ASM_INT_V({return js.objects.length});
      emp::pass_array_to_javascript(values);

	EM_ASM_ARGS({
	  var in_string = Pointer_stringify($1);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    var update_sel = js.objects[$0].data(emp.__incoming_array, fn);
	  } else {
	    var update_sel = js.objects[$0].data(emp.__incoming_array);
	  }
	  
	  js.objects.push(update_sel);
	},this->id, key);
     
      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }

    Selection EnterAppend(const char* type){
      /*This function appends the specified type of nodes to this
	selection's enter selection, which merges the enter selection
	with the update selection.

	Triggers an assertion error if this selection has no valid enter
	selection.
       */

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


    int GetAttrInt(const char* name){
      return EM_ASM_INT({
	  return js.objects[$0].attr(Pointer_stringify($1));
	}, this->id, name);
    }

    double GetAttrDouble(const char* name){
      return EM_ASM_DOUBLE({
	  return js.objects[$0].attr(Pointer_stringify($1));
	}, this->id, name);
    }

    int GetStyleInt(const char* name){
      return EM_ASM_INT({
	  return js.objects[$0].style(Pointer_stringify($1));
	}, this->id, name);
    }

    double GetStyleDouble(const char* name){
      return EM_ASM_INT({
	  return js.objects[$0].style(Pointer_stringify($1));
	}, this->id, name);
    }

    void SetText(const char* text){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(text, text)
    }

    void SetHtml(const char* value){
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(html, value)
    }    

    Selection Transition(const char* name=""){
      int new_id = EM_ASM_INT_V({return js.objects.length});
      EM_ASM_ARGS({
	  var transition = js.objects[$0].transition(Pointer_stringify($1));
	  js.objects.push(transition);
	}, this->id, name);
      return Selection(new_id);
    }    

    void Interrupt(const char* name=""){
      EM_ASM_ARGS({
	  js.objects[$0].interrupt(Pointer_stringify($1));
	}, this->id, name);
    }    

    Selection Insert(const char* name, const char* before=NULL){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      if (before){
	EM_ASM_ARGS({
	    var new = js.objects[$0].insert(Pointer_stringify($1), 
						  Pointer_stringify($2));
	    js.objects.push(new);
	  }, this->id, name, before);
      } else {
	EM_ASM_ARGS({
	    var new = js.objects[$0].insert(Pointer_stringify($1));
	    js.objects.push(new);
	  }, this->id, name);
      }
      return Selection(new_id);
    }    


    Selection EnterInsert(const char* name, const char* before=NULL){
      int new_id = EM_ASM_INT_V({return js.objects.length});

      emp_assert(this->enter);

      if (before){
	EM_ASM_ARGS({
	    var new = js.objects[$0].enter().insert(Pointer_stringify($1), 
						  Pointer_stringify($2));
	    js.objects.push(new);
	  }, this->id, name, before);
      } else {
	EM_ASM_ARGS({
	    var new = js.objects[$0].enter().insert(Pointer_stringify($1));
	    js.objects.push(new);
	  }, this->id, name);
      }
      this->enter = false;
      return Selection(new_id);
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

    void On(const char* type, const char* listener="null", bool capture=false){
   
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
	  
	}, this->id, type, listener, capture);
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


    char* GetAttrString(const char* name){
      int buffer = EM_ASM_INT({
	  var text = js.objects[$0].attr(Pointer_stringify($1));
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id, name);

      return (char *)buffer;
    }


    char* GetStyleString(const char* name){
      int buffer = EM_ASM_INT({
	  var text = js.objects[$0].style(Pointer_stringify($1));
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id, name);

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

    char* GetProperty(const char* name){
      int buffer = EM_ASM_INT({
	  var text = js.objects[$0].property(Pointer_stringify($1));
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id, name);

      return (char *)buffer;
    }

    void Sort(const char * comparator = "ascending"){
      /* Sort the selection by the given comparator function. The function
	 is indicated as a string and can either be in the d3 namespace,
	 the emp namespace (as results from JSWrapping C++ functions), or the
	 window namespace. These three options are checked sequentially in that
	 order, so a C++ function with the same name as d3 built-in will not
	 override the built-in. Similarly, a function declared directly in the
	 window will be overriden by a JSWrapped function with the same name.
       */
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(sort, comparator)
    }

    void Each(const char * function){
      /* Call the given function on each element of the selection. The function
	 is indicated as a string and can either be in the d3 namespace,
	 the emp namespace (as results from JSWrapping C++ functions), or the
	 window namespace. These three options are checked sequentially in that
	 order, so a C++ function with the same name as d3 built-in will not
	 override the built-in. Similarly, a function declared directly in the
	 window will be overriden by a JSWrapped function with the same name.
       */
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(each, function)
    }

    void Call(const char * function){
      /* Call the given function once on the entire selection. The function
	 is indicated as a string and can either be in the d3 namespace,
	 the emp namespace (as results from JSWrapping C++ functions), or the
	 window namespace. These three options are checked sequentially in that
	 order, so a C++ function with the same name as d3 built-in will not
	 override the built-in. Similarly, a function declared directly in the
	 window will be overriden by a JSWrapped function with the same name.

	 TODO: Allow arguments
       */
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(call, function)
    }

    Selection Filter(const char * selector){
      /* Returns a new selection, representing the current selection
	 filtered by the given string. The string can represent a function
	 in either the d3, emp, or window namespaces (as described in Sort),
	 or it can be a filter selector. Using a filter selector that somehow
	 has the same name as a function in one of the allowed namespaces
	 will create a problem, but shouldn't actually be possible.
       */

      int new_id = EM_ASM_INT_V({return js.objects.length});
      CALL_FUNCTION_THAT_ACCEPTS_FUNCTION_1_ARG(filter, selector)
      StoreNewObject();
      return Selection(new_id);
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
    this->id = EM_ASM_INT_V({return js.objects.length});
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

  Selection::Selection(int id){
    this->id = id;
    this->enter = false;
    this->exit = false;
  }

  /* I don't know why you'd actually want to do this
  Selection::Selection(Selection* selector, bool all){
    this->id = EM_ASM_INT_V({return js.objects.length});
    if (all){
      EM_ASM_ARGS({
	  js.objects[$0] = 
	  d3.selectAll($1.id)}, this->id, selector);
    }
    else {
      EM_ASM_ARGS({
	  js.objects[$0] = 
	  d3.select(Pointer_stringify($1))}, this->id, selector);
    }
  }
  */

//void bind_data(std::vector data){
// 
//}

  Selection Select(const char* selector){
    /*Slightly prettier UI for constructor*/
    return Selection(selector);
  }

  Selection SelectAll(const char* selector){
    /*Slightly prettier UI for constructor*/
    return Selection(selector, true);
  }

  template<std::size_t SIZE>
  Selection ShapesFromData(std::array<int32_t, SIZE> values, const char* shape){
    Selection s = Select("svg").SelectAll(shape).Data(values);
    s.EnterAppend(shape);
    return s;
  }

}

#endif
